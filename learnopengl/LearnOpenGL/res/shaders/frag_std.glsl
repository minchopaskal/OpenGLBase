#version 450 core

struct Material {
	sampler2D diffuse0;
	sampler2D diffuse1;
	sampler2D diffuse2;
	sampler2D specular0;
	sampler2D specular1;
	sampler2D emission;
	float shininess;
};

uniform Material material;

out vec4 fragColor;

in vec2 texCoords;

void main()
{
	fragColor = texture(material.diffuse0, texCoords);
}