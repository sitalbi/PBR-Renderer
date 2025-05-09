#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out mat3 TBN;
out vec3 ViewPos;
out mat4 VM;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	WorldPos = vec3(model * vec4(aPos, 1.0));

	ViewPos = vec3(view * model * vec4(aPos, 1.0));

	VM = view * model;

	TexCoords = aTexCoords;

	mat3 normalMatrix = mat3(transpose(inverse(model)));
	Normal = normalMatrix * aNormal;

	// TBN matrix
	vec3 T = normalize(normalMatrix * aTangent);
	vec3 B = normalize(normalMatrix * aBitangent);
	vec3 N = normalize(normalMatrix * aNormal);


	TBN = mat3(T, B, N);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
