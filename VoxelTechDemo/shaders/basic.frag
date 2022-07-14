#version 330 core

out vec4 FragColor;

in VS_OUT {
	vec3 texCoords;
	vec3 normal;
} fs_in;

uniform sampler2DArray arrTex;

void main() {
	FragColor = texture(arrTex, fs_in.texCoords);
	//FragColor = vec4(fs_in.normal, 1.0);
}