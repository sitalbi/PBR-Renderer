#version 450 core
in vec3 FragPos;
uniform vec3  lightPos;
uniform float farPlane;

void main()
{
    float lightDistance = length(FragPos - lightPos);

    gl_FragDepth = lightDistance / farPlane;
}
