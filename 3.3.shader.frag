#version 330 core
out vec4 fragColor;

struct Material{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct SpotLight{
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float cutOff;	// Inner Cone
	float outerCutOff;	// Outer Cone
};

struct DirLight{
	vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant;
	float linear;
	float quadratic;
};

#define NR_POINT_LIGHTS 4

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

uniform vec3 viewPos;
uniform Material material;
uniform SpotLight spotLight;
uniform DirLight dirLight;
uniform PointLight pointLight[NR_POINT_LIGHTS];

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir);

void main(){

	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 norm = normalize(normal);
	
	vec3 result = CalcDirLight(dirLight, norm, viewDir);
	result += CalcSpotLight(spotLight, norm, viewDir); 
	for(int i = 0; i < NR_POINT_LIGHTS; i++){
		result += CalcPointLight(pointLight[i], norm, fragPos, viewDir);
	}
	
	fragColor = vec4(result, 1.f);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){
	// calc diff & spec
	vec3 lightDir = normalize(-light.direction);
	float diff = max(0.f, dot(normal, lightDir));
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(0.f, dot(reflectDir, viewDir)), material.shininess);

	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));

	return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
	// calc diff & spec
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(0.f, dot(normal, lightDir));
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(0.f, dot(reflectDir, viewDir)), material.shininess);
	// ¼ÆËã¾àÀëË¥¼õ
	float distance = length(light.position - fragPos);
	float attenuation = 1.f / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord)) * attenuation;
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord)) * attenuation;
	vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord)) * attenuation;

	return ambient + diffuse + specular;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir){
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(0.f, dot(normal, lightDir));
	vec3 reflectDir = reflect(-lightDir,normal);
	float spec = pow(max(0.f,  dot(reflectDir, viewDir)), material.shininess);

	// SpotLight + ±ßÔµÈí»¯
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.f, 1.f);

	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord)) * intensity;
	vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord)) * intensity;

	return ambient + diffuse + specular;
}