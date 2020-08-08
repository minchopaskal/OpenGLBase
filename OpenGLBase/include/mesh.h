#ifndef MESH_H
#define MESH_H

#include "common_defines.h"

struct Mesh {
	Vec<Vec3> ps;
	Vec<int> ib;

	Vec<Vec3>& points();
	const Vec<Vec3>& points() const;
	const Vec<int>& indexBuffer() const;
};

// Returns a mesh with empty indexBuffer
Mesh* newDefaultCube();
bool deleteDefaultCube(Mesh* cube);

#endif // MESH_H