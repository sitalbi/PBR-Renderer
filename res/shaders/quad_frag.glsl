#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture; // The rendered scene color
uniform sampler2D ssaoTexture;    // The SSAO result (typically a grayscale occlusion factor)

uniform bool useSSAO; // Flag to enable/disable SSAO

void main()
{
    vec4 sceneColor = texture(screenTexture, TexCoords).rgba;
    float ssao = texture(ssaoTexture, TexCoords).r;
   
    vec4 finalColor = vec4(0.0);
    if (useSSAO) {
        finalColor = sceneColor * ssao;
    } else {
        finalColor = sceneColor;
    }
    FragColor = finalColor;
    
}
