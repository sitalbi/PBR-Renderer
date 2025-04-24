#version 450 core
layout(location = 0) out vec4 gColor;

in vec3 localPos;
uniform samplerCube environmentMap;

void main()
{    
     vec3 envColor = texture(environmentMap, localPos).rgb;

    gColor = vec4(envColor, 1.0);
}