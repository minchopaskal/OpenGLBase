#version 450 core

layout (location = 0) in vec3 aPos;

uniform mat4 VP; // view-projection matrix

out vec3 texCoords;

void main() {
	texCoords = aPos;
	const vec4 pos = VP * vec4(aPos, 1.0);
	gl_Position = pos.xyww;
}