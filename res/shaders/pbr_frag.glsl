#version 450 core

layout (location = 0) out vec4 gColor;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gPosition;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in mat3 TBN;
in vec3 ViewPos;
in mat4 VM;

uniform vec3 camPos;

// light parameters
uniform vec3 lightDir;
uniform vec3 lightColor;

#define MAX_POINT_LIGHTS 32

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;

    float constant;
    float linear;
    float quadratic;
    float farPlane;
};

uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform samplerCube pointShadowMaps[MAX_POINT_LIGHTS];

// PBR material
struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    vec3 emissiveColor;

    sampler2D albedoMap;
    sampler2D normalMap;
    sampler2D metallicMap;
    sampler2D roughnessMap;
    sampler2D aoMap;
    sampler2D emissiveMap;
};

uniform Material material;

// environment cubemap
uniform samplerCube irradianceMap;

// Specular IBL prefiltered map
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform sampler2D shadowMap;
uniform mat4     lightSpaceMatrix;

uniform float ambientIntensity;
uniform float lightIntensity;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    if(projCoords.z > 1.0) return 0.0; // outside of light frustum

    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r; 

    float currentDepth = projCoords.z;

    float bias = max(0.025 * (1.0 - dot(Normal, lightDir)), 0.0005); 

    //float shadow = currentDepth -bias > closestDepth  ? 1.0 : 0.0;

    // PCF 
    float shadow = 0.0;
    const float kernelSize = 4.0;
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfX = float(x) / kernelSize;
            float pcfY = float(y) / kernelSize;
            vec2 texCoords = projCoords.xy + vec2(pcfX, pcfY) * bias;
            float sampleDepth = texture(shadowMap, texCoords).r;
            shadow += currentDepth - bias > sampleDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0; // average the samples
    return shadow; 
}  

vec3 sampleOffsetDirections[20] = vec3[](
    vec3( 1, 1, 1), vec3(-1, 1, 1), vec3( 1,-1, 1), vec3(-1,-1, 1),
    vec3( 1, 1,-1), vec3(-1, 1,-1), vec3( 1,-1,-1), vec3(-1,-1,-1),
    vec3( 1, 0, 0), vec3(-1, 0, 0), vec3( 0, 1, 0), vec3( 0,-1, 0),
    vec3( 0, 0, 1), vec3( 0, 0,-1),
    vec3( 1, 1, 0), vec3(-1, 1, 0), vec3( 1,-1, 0), vec3(-1,-1, 0),
    vec3( 0, 1, 1), vec3( 0,-1, 1)
);

float ShadowPointLight(int index, vec3 fragPos, vec3 lightPos)
{
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);

    if (currentDepth >= pointLights[index].farPlane)
        return 0.0;

    float bias = 0.005 + 0.02 * (1.0 - dot(normalize(Normal), normalize(fragToLight)));
    bias = clamp(bias, 0.002, 0.015);


    // PCF sample
    float shadow = 0.0;
    int samples = 16;
    float diskRadius = 0.15;

    for (int i = 0; i < samples; ++i)
    {
        vec3 sampleDir = normalize(fragToLight + sampleOffsetDirections[i] * diskRadius);
        float closestDepth = texture(pointShadowMaps[index], sampleDir).r * pointLights[index].farPlane;

        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }

    shadow /= float(samples);
    return shadow;
}


void main()
{
    // Material properties calculation
    vec3 albedo = material.albedo *  pow(texture(material.albedoMap, TexCoords).rgb, vec3(2.2));

    float metallic = material.metallic * texture(material.metallicMap, TexCoords).r;

    float roughness = material.roughness * texture(material.roughnessMap, TexCoords).r;

    float ao = material.ao * texture(material.aoMap, TexCoords).r;

    vec3 emissive = material.emissiveColor * texture(material.emissiveMap, TexCoords).rgb;

    vec3 tangentNormal = texture(material.normalMap, TexCoords).rgb;
    tangentNormal = tangentNormal * 2.0 - 1.0;
    vec3 N = normalize(TBN * tangentNormal);

    vec3 viewSpaceN = normalize(mat3(VM) * N); // transform normal to view space for output gNormal

    vec3 V        = normalize(camPos - WorldPos);
    vec3 L        = normalize(lightDir);
    vec3 H        = normalize(V + L);
    float NdotL   = max(dot(N, L), 0.0);
    vec3 F0       = mix(vec3(0.04), albedo, metallic);

    // Cook-Torrance terms
    float  NDF  = DistributionGGX(N, H, roughness);
    float  G    = GeometrySmith(N, V, L, roughness);
    vec3   F    = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3   kS   = F;
    vec3   kD   = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3   spec = (NDF * G * F) / (4.0 * max(dot(N,V),0) * NdotL + 0.001);
    vec3   diff = kD * albedo / PI;

    // Shadow factor for this fragment
    float dirShadow = ShadowCalculation(lightSpaceMatrix * vec4(WorldPos,1.0));

    // Direct lighting 
    vec3 radiance = lightColor;
    vec3 directLighting = (diff + spec)  * radiance  * NdotL  * (1.0 - dirShadow) * lightIntensity;

    // Point light contribution
    for (int i = 0; i < numPointLights; ++i)
    {
        PointLight light = pointLights[i];
    
        vec3 L = normalize(light.position - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(light.position - WorldPos);
    
        // Attenuation
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        vec3 radiance = light.color * light.intensity * attenuation;
    
        float NdotL = max(dot(N, L), 0.0);
    
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    
        vec3 specular = (NDF * G * F) / (4.0 * max(dot(N, V), 0.0) * NdotL + 0.001);
        vec3 diffuse = kD * albedo / PI;
    
        float shadow = ShadowPointLight(i, WorldPos, light.position);
        directLighting += (diffuse + specular) * radiance * NdotL * (1.0 - shadow);

    }

    // IBL (ambient lighting)
    // Diffuse IBL
    vec3 irradiance    = texture(irradianceMap, N).rgb;
    vec3 diffuseIBL    = irradiance * albedo;

    // Specular IBL
    vec3 R             = reflect(-V, N);
    const float MAX_LOD = 4.0;
    vec3 prefiltered   = textureLod(prefilterMap, R, roughness * MAX_LOD).rgb;
    vec2 brdfLookup    = texture(brdfLUT, vec2(max(dot(N,V),0.0), roughness)).rg;
    vec3 specularIBL   = prefiltered * (F * brdfLookup.x + brdfLookup.y);

    // Pack into ambientLighting (modulated by AO)
    vec3 ambientLighting = (kD * diffuseIBL + specularIBL) * ao;


    // Final
    vec3 colorOut = directLighting + ambientLighting * ambientIntensity + emissive;


    //gColor = vec4(colorOut, 1.0);
    gColor = vec4(min(colorOut, vec3(10.0)), 1.0);
    gNormal = vec4(normalize(viewSpaceN), 1.0); // normal in view space
    gPosition = vec4(ViewPos, 1.0); // position in view space
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  