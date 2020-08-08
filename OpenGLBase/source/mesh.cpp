#include "mesh.h"

Vec<Vec3>& Mesh::points() {
	return ps;
}

const Vec<Vec3>& Mesh::points() const {
	return ps;
}

const Vec<int>& Mesh::indexBuffer() const {
	return ib;
}

Mesh* newDefaultCube() {
	unsigned short* data = (unsigned short*)malloc(sizeof(Mesh) + sizeof(unsigned short));
	data[0] = (unsigned short)0xBEBE; // indicate the mesh was created with this constructor
	Mesh* cube = new (data + 1) Mesh;

	cube->ps = {
		{ -0.5f, -0.5f, -0.5f },
		{  0.5f, -0.5f, -0.5f },
		{  0.5f,  0.5f, -0.5f },
		{  0.5f,  0.5f, -0.5f },
		{ -0.5f,  0.5f, -0.5f },
		{ -0.5f, -0.5f, -0.5f },

		{ -0.5f, -0.5f,  0.5f },
		{  0.5f, -0.5f,  0.5f },
		{  0.5f,  0.5f,  0.5f },
		{  0.5f,  0.5f,  0.5f },
		{ -0.5f,  0.5f,  0.5f },
		{ -0.5f, -0.5f,  0.5f },

		{ -0.5f,  0.5f,  0.5f },
		{ -0.5f,  0.5f, -0.5f },
		{ -0.5f, -0.5f, -0.5f },
		{ -0.5f, -0.5f, -0.5f },
		{ -0.5f, -0.5f,  0.5f },
		{ -0.5f,  0.5f,  0.5f },

		{  0.5f,  0.5f,  0.5f },
		{  0.5f,  0.5f, -0.5f },
		{  0.5f, -0.5f, -0.5f },
		{  0.5f, -0.5f, -0.5f },
		{  0.5f, -0.5f,  0.5f },
		{  0.5f,  0.5f,  0.5f },

		{ -0.5f, -0.5f, -0.5f },
		{  0.5f, -0.5f, -0.5f },
		{  0.5f, -0.5f,  0.5f },
		{  0.5f, -0.5f,  0.5f },
		{ -0.5f, -0.5f,  0.5f },
		{ -0.5f, -0.5f, -0.5f },

		{ -0.5f,  0.5f, -0.5f },
		{  0.5f,  0.5f, -0.5f },
		{  0.5f,  0.5f,  0.5f },
		{  0.5f,  0.5f,  0.5f },
		{ -0.5f,  0.5f,  0.5f },
		{ -0.5f,  0.5f, -0.5f },
	};

	return cube;
}

bool deleteDefaultCube(Mesh* cube) {
	unsigned short* data = reinterpret_cast<unsigned short*>(cube);
	data = data - 1;
	if (data[0] != 0xBEBE) {
		return false;
	}

	free(data);
	return true;
}