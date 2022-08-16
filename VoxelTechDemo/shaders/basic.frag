#version 330 core

out vec4 FragColor;

in VS_OUT {
	vec3 texCoords;
	vec3 normal;
	vec3 fragPos;
} fs_in;

uniform sampler2DArray arrTex;
uniform vec3 cameraPos;

void main() {
	// Darkness based on depth
	//vec3 lightDir = vec3(-0.5, -0.8, -0.2);
	//vec3 lightColor = vec3(1.0, 1.0, 0.9);
	//vec3 depthColor = vec3(0.0, 0.0, 0.0);
	//vec3 color = texture(arrTex, fs_in.texCoords).rgb;
	//vec3 litColor = mix(color, lightColor, clamp(dot(fs_in.normal, -lightDir) / 6.28, 0.0, 0.1));
	//vec3 finalColor = mix(depthColor, litColor, clamp(fs_in.fragCoords.y / 150, 0.5, 1.0));
	//FragColor = vec4(finalColor, 1.0);

	vec3 color = texture(arrTex, fs_in.texCoords).rgb;

	// Face-based lighting 
	// Ugly, but gets the job done for now. Will eventually transition to directional light
	vec3 darkness = vec3(0.0, 0.0, 0.0);
	if (fs_in.normal.x > 0) {
		color = mix(color, darkness, 0.2);
	}
	else if (fs_in.normal.z > 0) {
		color = mix(color, darkness, 0.15);
	}
	else if (fs_in.normal.x < 0) {
		color = mix(color, darkness, 0.1);
	}
	else if (fs_in.normal.z < 0) {
		color = mix(color, darkness, 0.15);
	}
	else if (fs_in.normal.y < 0) {
		color = mix(color, darkness, 0.6);
	}

	// Fog
	const vec3 FOG_COLOR = vec3(0.4, 0.7, 0.8);
	const int FOG_START_DIST = 150;
	const int FOG_FADE_DIST = 200;
	float dist = distance(cameraPos, fs_in.fragPos);
	if (dist > FOG_START_DIST) {
		color = mix(color, FOG_COLOR, clamp((dist - FOG_START_DIST) / FOG_FADE_DIST, 0.0, 1.0));
	}
	
	FragColor = vec4(color, 1.0);
	//FragColor = vec4(fs_in.normal, 1.0);
}