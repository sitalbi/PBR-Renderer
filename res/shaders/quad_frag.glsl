#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture; 
uniform sampler2D ssaoTexture;   

uniform bool useSSAO; // Flag to enable/disable SSAO

void main()
{
    vec4 sceneColor = texture(screenTexture, TexCoords).rgba;
    float ssao = texture(ssaoTexture, TexCoords).r;
   
    vec4 finalColor = vec4(0.0);
    if (useSSAO) {
        if(ssao != 0) {
            // Apply SSAO effect
            finalColor = vec4(sceneColor.rgb * ssao, sceneColor.a);
        } else {
            // If SSAO is zero, just use the scene color
            finalColor = sceneColor;
        }
    } else {
        finalColor = sceneColor;
    }

    FragColor = finalColor;
    
}
