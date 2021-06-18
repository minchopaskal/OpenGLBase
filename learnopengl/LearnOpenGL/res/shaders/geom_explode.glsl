#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
} gs_in[];

out GS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
} gs_out;

uniform bool explode;
uniform float explodeMagnitude;

vec3 getTriangleNormal() {
	vec3 e1 = vec3(gl_in[0].gl_Position - gl_in[1].gl_Position);
	vec3 e2 = vec3(gl_in[0].gl_Position - gl_in[2].gl_Position);
	return normalize(cross(e1, e2));
}

vec4 getNewPos(vec4 pos, vec3 normal) {
	return pos + explodeMagnitude * vec4(normal, 1.0);
}

void setOutParams(int idx, vec3 normal) {
	gs_out.fragPos = explode ? vec3(getNewPos(vec4(gs_in[idx].fragPos, 0.0), normal)) : gs_in[idx].fragPos;
	gs_out.normal = gs_in[idx].normal;
	gs_out.texCoords = gs_in[idx].texCoords;
}

void emitExplodedVertex(int idx, vec3 normal, bool explode) {
	const vec4 newPos = explode ? getNewPos(gl_in[idx].gl_Position, normal) : gl_in[idx].gl_Position;
	setOutParams(idx, normal);
	gl_Position = newPos;
	EmitVertex();
}

void main() {
	const vec3 norm = getTriangleNormal();

	emitExplodedVertex(0, norm, explode);
	emitExplodedVertex(1, norm, explode);
	emitExplodedVertex(2, norm, explode);

	EndPrimitive();
}