#version 450 core

out vec4 Color;

uniform vec3 color;

in vec3 pos;

void main()
{
	Color = vec4(pos, 1.f); 
}