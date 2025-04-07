#version 450 core
layout(location = 0) out vec4 gColor; 
layout(location = 1) out vec4 gNormal; 
layout(location = 2) out vec4 gPosition;  

in vec3 localPos;
uniform samplerCube environmentMap;

void main()
{    
     vec3 envColor = texture(environmentMap, localPos).rgb;
    
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 

    // Only output to color attachment as the skybox data should not be passed to the SSAO passes
    gColor = vec4(envColor, 1.0);
    gNormal = vec4(0.0);                 
    gPosition = vec4(0.0);
}