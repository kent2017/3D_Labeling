#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 color;

out vec4 frag_color;

//uniform vec3 objectColor;
//uniform vec3 lightPos;

void main() {
	vec3 lightPos = vec3(10, 0, 1000);
	vec3 lightColor = vec3(1.0, 1.0, 1.0);

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);

	vec3 ambient = vec3(0.1, 0.1, 0.1);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	vec3 result = (ambient + diffuse) * color;

	frag_color = vec4(result, 1.0f);
}
