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
	float cutOff;
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

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

uniform vec3 viewPos;
uniform Material material;
uniform SpotLight light;

void main(){
	// load by texture_diffuse
	vec3 diffuse_t = texture(material.diffuse, texCoord).rgb;
	// load by texture_specular
	vec3 specular_t = texture(material.specular, texCoord).rgb;

	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(light.position - fragPos);

	// SpotLight:
	float theta = dot(lightDir, normalize(-light.direction));
	if(theta > light.cutOff){
		// ambient
		vec3 ambient = diffuse_t * light.ambient;

		// diffuse
		float diff = max(dot(norm, lightDir), 0.f);
		vec3 diffuse = (diff * diffuse_t) * light.diffuse;

		// specular
		vec3 viewDir = normalize(viewPos - fragPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(0.f, dot(viewDir, reflectDir)), material.shininess);
		vec3 specular = (spec * specular_t) * light.specular;
	
		vec3 result = ambient + diffuse + specular;
		fragColor = vec4(result, 1.f);
	}
	else{
		fragColor = vec4(light.ambient * diffuse_t, 1.f);
	}
}