#version 450 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D sceneColor;
uniform float threshold;      // ≈ 1.0 typical
uniform float softThreshold;  // ≈ 0.8–0.95 typical

void main()
{
    vec3 color = texture(sceneColor, TexCoords).rgb;
    
    // Standard luminance
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));

    // Soft threshold using smoothstep-style rolloff
    float knee = threshold * softThreshold;
    float curveStart = threshold - knee;
    float curveEnd   = threshold + knee;

    float t = smoothstep(curveStart, curveEnd, luminance);

    // Apply intensity curve
    vec3 bright = color * t;

    FragColor = vec4(bright, 1.0);
}
