#version 330 core

struct Material{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

out vec4 fragColor;
in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

uniform sampler2D tex;
uniform vec3 viewPos;
uniform Material material;
uniform Light light;


void main(){
	vec4 objColor = texture(tex, texCoord);
	// vec4 objColor = vec4(1.f);

	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(light.position - fragPos);
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	// ambient
	vec3 ambient = material.ambient * light.ambient;

	// diffuse
	float diff = max(dot(norm, lightDir), 0.f);
	vec3 diffuse = diff * material.diffuse * light.diffuse;

	//specular
	float spec = pow(max(0.f, dot(viewDir, reflectDir)), material.shininess);
	vec3 specular = spec * material.specular * light.specular;

	vec3 result = (ambient + diffuse + specular) * vec3(objColor.xyz);
	fragColor = vec4(result, 1.f);
}