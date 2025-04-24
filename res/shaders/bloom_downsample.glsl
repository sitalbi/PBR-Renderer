#version 450 core

// Downsampling on a texture, taken from Sledgehammer Games presentation at ACM Siggraph 2014

layout (location = 0) out vec4 downsample;

in vec2 TexCoords;

uniform sampler2D srcTexture;
uniform vec2 srcResolution;

void main()
{
    vec2 srcTexelSize = 1.0 / srcResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    vec3 a = texture(srcTexture, vec2(TexCoords.x - 2*x, TexCoords.y + 2*y)).rgb;
    vec3 b = texture(srcTexture, vec2(TexCoords.x,       TexCoords.y + 2*y)).rgb;
    vec3 c = texture(srcTexture, vec2(TexCoords.x + 2*x, TexCoords.y + 2*y)).rgb;

    vec3 d = texture(srcTexture, vec2(TexCoords.x - 2*x, TexCoords.y)).rgb;
    vec3 e = texture(srcTexture, vec2(TexCoords.x,       TexCoords.y)).rgb;
    vec3 f = texture(srcTexture, vec2(TexCoords.x + 2*x, TexCoords.y)).rgb;

    vec3 g = texture(srcTexture, vec2(TexCoords.x - 2*x, TexCoords.y - 2*y)).rgb;
    vec3 h = texture(srcTexture, vec2(TexCoords.x,       TexCoords.y - 2*y)).rgb;
    vec3 i = texture(srcTexture, vec2(TexCoords.x + 2*x, TexCoords.y - 2*y)).rgb;

    vec3 j = texture(srcTexture, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;
    vec3 k = texture(srcTexture, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;
    vec3 l = texture(srcTexture, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 m = texture(srcTexture, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    vec3 color = e*0.125;
    color += (a+c+g+i)*0.03125;
    color += (b+d+f+h)*0.0625;
    color += (j+k+l+m)*0.125;

    downsample = vec4(color, 1.0);
}