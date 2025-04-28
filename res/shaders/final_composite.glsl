#version 450 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D backgroundTexture;  // The skybox texture
uniform sampler2D sceneTexture;       // The scene with lighting applied
uniform sampler2D bloomTexture;       // The blurred bloom texture
uniform float exposure;               // Exposure value for tone mapping

uniform bool useBloom; // Flag to enable/disable bloom effect

void main()
{
    // Retrieve all textures
    vec4 backgroundColor = texture(backgroundTexture, TexCoords);
    vec4 sceneColor = texture(sceneTexture, TexCoords);
    vec3 bloomColor = texture(bloomTexture, TexCoords).rgb * 0.25; // Adjust bloom intensity
    
    // Blend scene over background
    vec3 combinedColor = mix(backgroundColor.rgb, sceneColor.rgb, sceneColor.a);

    // Apply bloom if enabled
    if (useBloom) {
        combinedColor += bloomColor;
    }
    
    // Apply HDR tone mapping to combined scene and bloom
    vec3 hdrCombined = combinedColor;
    vec3 toneMappedScene = vec3(1.0) - exp(-hdrCombined * exposure);
    
    vec3 result = toneMappedScene;
    
    // Gamma correction
    result = pow(result, vec3(1.0 / 2.2));
    
    FragColor = vec4(result, 1.0);
}