#ifndef MESH_H
#define MESH_H

#include "common_defines.h"

struct Mesh {
	static const int TRI_FACE_VERTS = 3;
	static const int QUAD_FACE_VERTS = 4;

	Vec<Vec3> ps; // Vertices
	Vec<Vec3i> triFaces; // Triangular faces
	Vec<Vec4i> faces; // Quadrangular faces
	bool subdivided = false; // True iff the mesh was subdivided

public:
	void importMesh() { __TODO__ }
	void subdivide();

	Vec<Vec3>& points();
	const Vec<Vec3>& points() const;

private:
	void triangulate(); // turn quad faces into triangular faces. Used after subdivion.

	friend Mesh* newDefaultCube();
};

// Returns a mesh with empty indexBuffer
Mesh* newDefaultCube();
bool deleteDefaultCube(Mesh *&cube);

#endif // MESH_H