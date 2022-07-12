#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 aInstancedModel; // Instanced array

out vec3 normal;
out vec2 texCoord;
out vec3 FragPos;

uniform mat4 view;
uniform mat4 projection;

void main() {
	gl_Position = projection * view * aInstancedModel * vec4(aPos, 1.0);
	FragPos = vec3(aInstancedModel * vec4(aPos, 1.0));
	texCoord = aTexCoord;
	normal = mat3(transpose(inverse(aInstancedModel))) * aNormal;
}