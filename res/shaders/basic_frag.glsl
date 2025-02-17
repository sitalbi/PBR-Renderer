#version 330 core
out vec4 FragColor;

in vec3 Normal;

void main() {

	// different color for each face of the cube
	vec3 color = Normal * 0.5 + 0.5;
	FragColor = vec4(color, 1.0);
	
}
