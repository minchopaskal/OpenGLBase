#include "mesh.h"

#include "common_headers.h"
#include "shader.h"

Mesh::Mesh() : VAO(-1) { }

void Mesh::init(Vec<Vertex> &v, Vec<unsigned int> &i, const Material &m) {
	vertices = std::move(v);
	indices = std::move(i);
	material = m;
	setupMesh();
}

// TODO: make RAII somehow. SharedPtrs?!
void Mesh::deinit() {
	vertices.clear();
	indices.clear();

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(2, buffers);
}

void Mesh::draw(Shader &shader) const {
	for (int i = 0; i < MaterialField::MF_SHININESS; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		shader.setInt(material.getFieldName(i), i); // set ith sampler to correspond to ith texture unit
		glBindTexture(GL_TEXTURE_2D, material.getIntField(i).get());
	}
	shader.setField(material, MF_SHININESS, materialField2Type[MF_SHININESS]);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

Handle Mesh::getHandle() const {
	return VAO;
}

void Mesh::setupMesh() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(2, buffers);

	glBindVertexArray(VAO);
	
	const int VBO = buffers[0];
	const int IBO = buffers[1];

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoords));

	glBindVertexArray(0);
}
