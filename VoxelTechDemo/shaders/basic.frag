#version 330 core

out vec4 FragColor;

in VS_OUT {
	vec3 texCoords;
	vec3 normal;
	vec3 fragCoords;
} fs_in;

uniform sampler2DArray arrTex;

void main() {
	vec3 lightDir = vec3(-0.5, -0.8, -0.2);
	vec3 lightColor = vec3(1.0, 1.0, 0.9);
	vec3 depthColor = vec3(0.0, 0.0, 0.0);
	vec3 color = texture(arrTex, fs_in.texCoords).rgb;
	vec3 litColor = mix(color, lightColor, clamp(dot(fs_in.normal, -lightDir) / 6.28, 0.0, 0.1));
	vec3 finalColor = mix(depthColor, litColor, clamp(fs_in.fragCoords.y / 150, 0.5, 1.0));

	FragColor = vec4(finalColor, 1.0);
	
	//FragColor = vec4(fs_in.normal, 1.0);
}