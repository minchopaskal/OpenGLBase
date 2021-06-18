#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in mat4 aTransform;

layout(std140, binding=0) uniform VP {
	mat4 projection;
	mat4 view;
};

out VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
} vs_out;

void main() {
	gl_Position = projection * view * aTransform * vec4(aPos, 1.0);

	const mat4 normalMat = transpose(inverse(aTransform));

	vs_out.fragPos = vec3(aTransform * vec4(aPos,1.0));
	vs_out.normal = normalize(vec3(normalMat * vec4(aNormal, 1.0)));
	vs_out.texCoords = aTexCoord;
}
