#version 450 core

layout (triangles) in;
layout (line_strip, max_vertices=6) out;

in VS_OUT {
	vec3 normal;
} gs_in[];

vec3 getTriangleNormal(int idx) {
	return gs_in[idx].normal;
}

uniform mat4 proj;

void emitNormal(int idx) {
	gl_Position = proj * gl_in[idx].gl_Position;
	EmitVertex();

	gl_Position = proj * (gl_in[idx].gl_Position + vec4(gs_in[idx].normal, 1.0) * .1f);
	EmitVertex();
	
	EndPrimitive();
}

void main() {
	emitNormal(0);
	emitNormal(1);
	emitNormal(2);

}