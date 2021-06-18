#ifndef MESH_H
#define MESH_H

#include "common_defines.h"
#include "drawable.h"
#include "material.h"

struct Shader;

struct Vertex {
	Vec3 position;
	Vec3 normal;
	Vec2 texCoords;
};

struct Mesh : DrawableInterface {
	Vec<Vertex> vertices;
	Vec<unsigned int> indices;
	Material material;

	Mesh();

	void init(Vec<Vertex> &v, Vec<unsigned int> &i, const Material &m);
	void deinit();
	void draw(Shader &shader) const override;

	Handle getHandle() const;
private:
	Handle VAO;
	Handle buffers[2];

	void setupMesh();
};

#endif // MESH_H