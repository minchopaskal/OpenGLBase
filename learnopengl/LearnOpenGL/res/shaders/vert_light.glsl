#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout(std140, binding=0) uniform VP {
	mat4 projection;
	mat4 view;
};

out VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
} vs_out;

uniform mat4 modelMat;
uniform mat4 normalMat;

void main() {
	gl_Position = projection * view * modelMat * vec4(aPos, 1.0); 
	vs_out.fragPos = vec3(modelMat * vec4(aPos, 1.0));
	vs_out.normal = vec3(normalMat * vec4(aNormal, 0.0));
	vs_out.texCoords = aTexCoords;
}