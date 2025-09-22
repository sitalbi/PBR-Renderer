#version 450 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D backgroundTexture;  
uniform sampler2D sceneTexture;       
uniform sampler2D bloomTexture;       
uniform float exposure;               

uniform bool useBloom; 

void main()
{
    vec4 backgroundColor = texture(backgroundTexture, TexCoords);
    vec4 sceneColor = texture(sceneTexture, TexCoords);
    vec3 bloomColor = texture(bloomTexture, TexCoords).rgb * 0.25;
    
    vec3 combinedColor = mix(backgroundColor.rgb, sceneColor.rgb, sceneColor.a);

    if (useBloom) {
        combinedColor += bloomColor;
    }
    
    // Tone mapping
    vec3 hdrCombined = combinedColor;
    vec3 toneMappedScene = vec3(1.0) - exp(-hdrCombined * exposure);
    
    vec3 result = toneMappedScene;
    
    // Gamma correction
    result = pow(result, vec3(1.0 / 2.2));
    
    FragColor = vec4(result, 1.0);
}