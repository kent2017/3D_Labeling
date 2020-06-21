#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;

uniform mat4 projMat;
uniform mat4 viewMat;
uniform mat4 modelMat;

out vec3 Normal;
out vec3 FragPos;

void main(){
	gl_Position = projMat * viewMat * modelMat * vec4(vPosition, 1);
	FragPos = vec3(modelMat * vec4(vPosition, 1.0f));
	Normal = mat3(transpose(inverse(modelMat))) * vNormal;
}

