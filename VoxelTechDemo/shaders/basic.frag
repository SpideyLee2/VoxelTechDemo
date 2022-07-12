#version 330 core

out vec4 FragColor;

in VS_OUT {
	vec2 texCoords;
	vec3 normal;
} fs_in;

uniform sampler2D diffuseMap;

void main() {
	FragColor = texture(diffuseMap, fs_in.texCoords);
	//FragColor = vec4(fs_in.normal, 1.0);
}