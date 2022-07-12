#version 330 core

struct Material {
	sampler2D diffuse;
	sampler2D specular;

	float shininess;
};
uniform Material material;

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
#define NUM_DIR_LIGHTS 2
uniform DirLight dirLight[NUM_DIR_LIGHTS];

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
#define NUM_POINT_LIGHTS 4
uniform PointLight pointLight[NUM_POINT_LIGHTS];

struct SpotLight {
	bool isActive;
	vec3 position;
	vec3 direction;

	float innerCutoff;
	float outerCutoff;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform SpotLight spotLight;

in vec2 texCoord;
in vec3 normal;
in vec3 FragPos;

uniform vec3 viewPos;

out vec4 FragColor;

vec3 calcDirLight(DirLight light, vec3 normal);
vec3 calcPointLight(PointLight light, vec3 normal);
vec3 calcSpotLight(vec3 normal);

void main() {
	vec3 norm = normalize(normal);
	// For moonlight, add slight dark blue offset to color
	vec3 color = vec3(0.0);
	for (int i = 0; i < NUM_DIR_LIGHTS; ++i) {
		color += calcDirLight(dirLight[i], norm);
	}
	for (int i = 0; i < NUM_POINT_LIGHTS; ++i) {
		color += calcPointLight(pointLight[i], norm);
	}
	color += calcSpotLight(norm);
	
	FragColor = vec4(color, 1.0);
}

vec3 calcDirLight(DirLight light, vec3 normal) {
	vec3 lightDir = normalize(light.direction);

	vec3 ambient = light.ambient * texture(material.diffuse, texCoord).rgb;

	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoord).rgb;

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.specular, texCoord).rgb;

	return ambient + diffuse + specular;
}

vec3 calcPointLight(PointLight light, vec3 normal) {
	vec3 lightDir = normalize(light.position - FragPos);

	float d = distance(light.position, FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * d + light.quadratic * (d * d));

	vec3 ambient = light.ambient * texture(material.diffuse, texCoord).rgb * attenuation;
	
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoord).rgb * attenuation;

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.specular, texCoord).rgb * attenuation;

	return ambient + diffuse + specular;
}

vec3 calcSpotLight(vec3 normal) {
	if (spotLight.isActive) {
		vec3 lightDir = normalize(spotLight.position - FragPos);
		vec3 spotDir = normalize(spotLight.direction);

		float theta = dot(-spotDir, lightDir);
		float intensity = clamp((theta - spotLight.outerCutoff) / (spotLight.innerCutoff - spotLight.outerCutoff), 0.0, 1.0);

		float d = distance(spotLight.position, FragPos);
		float attenuation = 1.0 / (spotLight.constant + spotLight.linear * d + spotLight.quadratic * (d * d));

		vec3 ambient = spotLight.ambient * texture(material.diffuse, texCoord).rgb * attenuation;
	
		float diff = max(dot(lightDir, normal), 0.0);
		vec3 diffuse = spotLight.diffuse * diff * texture(material.diffuse, texCoord).rgb * attenuation * intensity;

		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		vec3 specular = spotLight.specular * spec * texture(material.specular, texCoord).rgb * attenuation * intensity;

		return ambient + diffuse + specular;
	}
	else {
		return vec3(0.0);
	}
}