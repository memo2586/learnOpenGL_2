#version 330 core

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 nrmMat;

out vec3 normal;
out vec3 fragPos;
out vec2 texCoord;

void main(){
	gl_Position = projection * view * model * vec4(vertPos, 1.f);
	fragPos = vec3(model * vec4(vertPos, 1.f));

	vec4 nrm = nrmMat * vec4(vertNormal, 1.f);
	normal = vec3(nrm.xyz);
	texCoord = aTexCoord;
}