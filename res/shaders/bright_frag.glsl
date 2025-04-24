#version 450 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D sceneColor;  // The HDR rendered scene
uniform float threshold;       // Brightness threshold
uniform float softThreshold;   // Softness of the threshold (0.0-1.0, try 0.2)

void main()
{
    // Retrieve the color from the scene
    vec3 color = texture(sceneColor, TexCoords).rgb;
    
    // Compute luminance 
    vec3 luminances = vec3(0.2126, 0.7152, 0.0722); 

    float luminance = dot(color, luminances);
    
    // Apply soft threshold
    float knee = threshold * softThreshold;
    float soft = luminance - threshold + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = soft * soft / (4.0 * knee + 0.00001);
    
    // Calculate contribution based on threshold 
    float contribution = max(soft, luminance - threshold);
    contribution /= max(luminance, 0.00001);
    
    // Apply the contribution to the color
    vec3 brightColor = color * contribution;
    
    FragColor = vec4(brightColor, 1.0);
}