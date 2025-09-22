#version 450 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D sceneColor;
uniform float threshold;     
uniform float softThreshold; 

void main()
{
    vec3 color = texture(sceneColor, TexCoords).rgb;
    
    vec3 luminances = vec3(0.2126, 0.7152, 0.0722); 

    float luminance = dot(color, luminances);
    
    // Apply soft threshold
    float knee = threshold * softThreshold;
    float soft = luminance - threshold + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = soft * soft / (4.0 * knee + 0.00001);
    
    float contribution = max(soft, luminance - threshold);
    contribution /= max(luminance, 0.00001);
    
    vec3 brightColor = color * contribution;
    
    FragColor = vec4(brightColor, 1.0);
}