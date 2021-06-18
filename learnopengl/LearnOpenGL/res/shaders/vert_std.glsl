#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout(std140, binding=0) uniform VP {
	mat4 projection;
	mat4 view;
};

uniform mat4 modelMat;
uniform mat4 normalMat;

out VS_OUT {
	vec3 normal;
} vs_out;

void main() {
	gl_Position = view * modelMat * vec4(aPos, 1.0f);
	vs_out.normal = vec3(normalize((transpose(inverse(view)) * normalMat) * vec4(aNormal, 1.0)));
}