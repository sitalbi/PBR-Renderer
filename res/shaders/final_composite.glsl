#version 450 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D backgroundTexture;  
uniform sampler2D sceneTexture;       
uniform sampler2D bloomTexture;       
uniform float exposure;               

uniform bool useBloom; 

uniform int toneMappingMode; 

vec3 defaultToneMapping(vec3 x)
{
    vec3 hdrCombined = x;
    vec3 toneMappedScene = vec3(1.0) - exp(-hdrCombined * exposure);
    
    return toneMappedScene;
}

vec3 acesToneMapping(vec3 x)
{
    x *= exposure;

    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 reinhardToneMapping(vec3 x)
{
    return x / (x + vec3(1.0));
}

vec3 applyToneMapping(vec3 color)
{
    if (toneMappingMode == 0)
        return defaultToneMapping(color);
    else if (toneMappingMode == 1)
        return reinhardToneMapping(color);
    else
        return acesToneMapping(color);
}

void main()
{
    vec4 backgroundColor = texture(backgroundTexture, TexCoords);
    vec4 sceneColor = texture(sceneTexture, TexCoords);
    vec3 bloomColor = texture(bloomTexture, TexCoords).rgb * 0.25;
    
    vec3 combinedColor = mix(backgroundColor.rgb, sceneColor.rgb, sceneColor.a);

    if (useBloom) {
        combinedColor += bloomColor;
    }
    
    vec3 result = applyToneMapping(combinedColor);
    
    // Gamma correction
    result = pow(result, vec3(1.0 / 2.2));
    
    FragColor = vec4(result, 1.0);
}
