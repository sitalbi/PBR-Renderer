#version 450 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in mat3 TBN;

uniform vec3 camPos;

// light parameters
uniform vec3 lightDir;
uniform vec3 lightColor;

// material parameters
uniform vec3 uAlbedo;
uniform float uMetallic;
uniform float uRoughness;
uniform float uAo;

// material textures 
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

// material booleans 
uniform bool useAlbedoMap;
uniform bool useNormalMap;
uniform bool useMetallicMap;
uniform bool useRoughnessMap;
uniform bool useAoMap;

// environment cubemap
uniform samplerCube irradianceMap;

// Specular IBL prefiltered map
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

void main()
{

	vec3 albedo = uAlbedo;
//	if(useAlbedoMap) {
//		albedo = texture(albedoMap, TexCoords).rgb;
//	}
	float metallic = uMetallic;
//	if(useMetallicMap) {
//		metallic = texture(metallicMap, TexCoords).r;
//	}
	float roughness = uRoughness;
//	if(useRoughnessMap){
//		roughness = texture(roughnessMap, TexCoords).r;
//	}
	float ao = uAo;
//	if(useAoMap) {
//		ao = texture(aoMap, TexCoords).r;
//	}

	vec3 N= normalize(Normal);  
//	if (useNormalMap)
//    {
//		// Get normal in tangent space
//        vec3 tangentNormal = texture(normalMap, TexCoords).rgb;
//		// from range [0..1] to [-1..1]
//        tangentNormal = tangentNormal * 2.0 - 1.0;  
//
//        // Transform normal to world space
//        N = normalize(TBN * tangentNormal);
//    }
//    else
//    {
//        N = normalize(Normal);  
//    }

	vec3 V = normalize(camPos - WorldPos);
    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    // reflectance equation
    vec3 Lo = vec3(0.0);
    
    // calculate per-light radiance
    vec3 L = normalize(lightDir);
    vec3 H = normalize(V + L);
    //float distance = length(lightPos - WorldPos);     // point light
    //float attenuation = 1.0 / (distance * distance);  // point light
    float attenuation = 1.0;
    vec3 radiance = lightColor * attenuation;
    
    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(N, V), 0.0), F0);
    
    // reflectance
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    // specular
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular     = numerator / denominator;  
    
    float NdotL = max(dot(N, L), 0.0);                
    Lo = (kD * albedo / PI + specular) * radiance * NdotL; 

    // diffuse IBL
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse    = irradiance * albedo;

    // specular IBL
    vec3 R = reflect(-V, N);
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;   
    vec2 envBRDF  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specularIBL = prefilteredColor * (F*envBRDF.x + envBRDF.y);

    vec3 ambient    = (kD*diffuse + specularIBL) * ao; 

    vec3 color = ambient + Lo;
    
    // gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);

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