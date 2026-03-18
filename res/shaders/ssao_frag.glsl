#version 450 core

out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;  
uniform sampler2D gNormal;    
uniform sampler2D noiseTexture;   


const int kernelSize = 64; 
uniform vec3 samples[kernelSize];


uniform mat4 projection;     
uniform vec2 noiseScale;

const float radius = 0.5;

void main()
{
    // Retrieve the fragment's position and normal from the G-buffer
    vec3 fragPos = texture(gPosition, TexCoords).rgb; // Position in view space
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb); // Normal in view space
    
    // Get a random vector from the noise texture in the range [-1,1]
    vec3 randomVec = normalize(texture(noiseTexture, TexCoords * noiseScale).xyz * 2.0 - 1.0);
    
    // Create a TBN matrix (tangent, bitangent, normal) to reorient the sample kernel
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    
    // Occlusion factor calculation
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        vec3 sampleVec = TBN * samples[i];
        sampleVec = fragPos + sampleVec * radius;
        
        vec4 offset = projection * vec4(sampleVec, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        
        float sampleDepth = texture(gPosition, offset.xy).z;
        
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= sampleVec.z + 0.001 ? 1.0 : 0.0) * rangeCheck;
    }
    
    occlusion = 1.0 - (occlusion / float(kernelSize));
    FragColor = occlusion;
}
