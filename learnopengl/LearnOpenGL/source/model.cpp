#include "model.h"

#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "common_headers.h"
#include "shader.h"
#include "utility.h"

Model::~Model() {
	deinit();
}

void Model::deinit() {
	for (int i = 0; i < meshes.size(); ++i) {
		meshes[i].deinit();
	}
}

void Model::draw(Shader &shader) const {
	for (int i = 0; i < meshes.size(); ++i) {
		meshes[i].draw(shader);
	}
}

void Model::loadModel(const String &path) {
	stbi_set_flip_vertically_on_load(true);

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		printf("ASSIMP::ERROR::%s", importer.GetErrorString());
		return;
	}
	directory = path.substr(0, path.find_last_of('\\'));

	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
		processMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		processNode(node->mChildren[i], scene);
	}
}

void Model::processMesh(aiMesh *mesh, const aiScene *scene) {
	Vec<Vertex> vertices;
	Vec<unsigned int> indices;
	Material mat("material");

	vertices.resize(mesh->mNumVertices);
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		Vertex v;

		Vec3 &pos = v.position;
		pos.x = mesh->mVertices[i].x;
		pos.y = mesh->mVertices[i].y;
		pos.z = mesh->mVertices[i].z;
		
		Vec3 &n = v.normal;
		if (mesh->HasNormals()) {
			n.x = mesh->mNormals[i].x;
			n.y = mesh->mNormals[i].y;
			n.z = mesh->mNormals[i].z;
		} else {
			n = Vec3(0.f);
		}

		Vec2 &t = v.texCoords;
		if (mesh->HasTextureCoords(0)) {
			t.x = mesh->mTextureCoords[0][i].x;
			t.y = mesh->mTextureCoords[0][i].y;
		} else {
			t = Vec2(0.f);
		}

		vertices[i] = v;
	}

	indices.resize(mesh->mNumFaces * 3);
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		const aiFace &face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
			indices[i * 3 + j] = face.mIndices[j];
		}
	}

	if (mesh->mMaterialIndex >= 0) {
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
		loadMaterialTexturesForMesh(mat, material, aiTextureType_DIFFUSE);
		loadMaterialTexturesForMesh(mat, material, aiTextureType_SPECULAR);
		loadMaterialTexturesForMesh(mat, material, aiTextureType_EMISSIVE);
		loadMaterialTexturesForMesh(mat, material, aiTextureType_SHININESS);
	}

	meshes.push_back({});
	meshes.back().init(vertices, indices, mat);
}

Handle loadTexture(const String &path) {
	Handle texID;
	glGenTextures(1, &texID);

	int w, h, ncomp;
	unsigned char *data = stbi_load(path.c_str(), &w, &h, &ncomp, 0);

	if (!data) {
		printf("Texture %s load failed!", path.c_str());
	} else {
		static GLenum formatMap[4] = { GL_RED, GL_RED, GL_RGB, GL_RGBA };
		GLenum format = formatMap[ncomp - 1];

		glBindTexture(GL_TEXTURE_2D, texID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		GLint wrapMode = format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	stbi_image_free(data);
	return texID;
}

void Model::loadMaterialTexturesForMesh(Material &material, aiMaterial *mat, aiTextureType type) {
	const static Map<aiTextureType, MaterialField> texTypeMap = {
		{ aiTextureType_DIFFUSE, MF_DIFFUSE0 },
		{ aiTextureType_SPECULAR, MF_SPECULAR0 },
		{ aiTextureType_EMISSIVE, MF_EMISSION },
		{ aiTextureType_SHININESS, MF_SHININESS },
	};

	const static Map<aiTextureType, int> texTypeMaxCount = {
		{ aiTextureType_DIFFUSE, 3 },
		{ aiTextureType_SPECULAR, 2 },
		{ aiTextureType_EMISSIVE, 1 },
		{ aiTextureType_SHININESS, 1 },
	};
	
	unsigned int texCount = Min(mat->GetTextureCount(type), (unsigned int)(texTypeMaxCount.at(type)));
	for (unsigned int i = 0; i < texCount; i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		size_t hash = getStringHash(str.C_Str());
		
		Texture tex;
		if (textureHashes.find(hash) != textureHashes.end()) {
			tex = textureHashes.at(hash);
		} else {
			tex.id = loadTexture(directory + "\\" + String(str.C_Str()));
			textureHashes[hash] = tex;
		}

		MaterialField mfType = texTypeMap.at(type);
		switch (mfType) {
		case MF_DIFFUSE0:
			material.diffuse[i] = tex;
			break;
		case MF_SPECULAR0:
			material.specular[i] = tex;
			break;
		case MF_EMISSION:
			material.emission = tex;
			break;
		case MF_SHININESS:
			mat->Get(AI_MATKEY_SHININESS, material.shininess);
			break;
		}
	}
}

void instanceUpdater(DrawableInterface *obj, UpdateParams params) {
	Instance *i = dynamic_cast<Instance*>(obj);
	InstanceUpdateParams *p = reinterpret_cast<InstanceUpdateParams *>(params);
	i->position = p->position;
	i->scale = p->scale;
}

Instance::Instance() : 
	model(nullptr), 
	outlineShader(nullptr),
	position(0.f),
	scale(1.f), 
	inited(false), 
	outlined(false) {
	DrawableInterface::f = instanceUpdater;
}

void Instance::init(Model *model, bool outlined, Shader *outlineShader) {
	if (inited) {
		return;
	}
	inited = true;
	this->model = model;
	this->outlined = outlined;
	if (!outlined) {
		return;
	}
	if (!outlineShader) {
		inited = false;
		model = nullptr;
		return;
	}
	this->outlineShader = outlineShader;
}

void Instance::draw(Shader &shader) const {
	if (model == nullptr) {
		return;
	}

	if (outlined) {
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
	}

	auto modelMat = Mat4(1.f);
	modelMat = glm::translate(modelMat, position);
	modelMat = glm::scale(modelMat, scale);
	shader.setMat4("modelMat", modelMat);
	shader.setMat4("normalMat", glm::transpose(glm::inverse(modelMat)));
	model->draw(shader);

	if (!outlined) {
		return;
	}

	glDisable(GL_DEPTH_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);

	outlineShader->use();
	modelMat = Mat4(1.f);
	modelMat = glm::translate(modelMat, position);
	modelMat = glm::scale(modelMat, 1.02f * scale);
	outlineShader->setMat4("modelMat", modelMat);
	model->draw(*outlineShader);

	glEnable(GL_DEPTH_TEST);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	shader.use();
}

InstancedModel::InstancedModel() :
	model(nullptr),
	transformsBuffer(-1),
	instanceCount(0) { }

void InstancedModel::init(const String &modelPath, const Vec<Mat4> &transformations, int instanceCount) {
	deinit();
	
	Model::init(modelPath);

	updateTransformations(transformations, instanceCount);
}

void InstancedModel::init(Model *model, const Vec<Mat4> &transformations, int instanceCount) {
	deinit();
	
	this->model = model;
	
	updateTransformations(transformations, instanceCount);
}

void InstancedModel::deinit() {
	Model::deinit();
	if (model) {
		model->deinit();
	}
	glDeleteBuffers(1, &transformsBuffer);

	model = nullptr;
	transformsBuffer = -1;
	instanceCount = 0;
}

void InstancedModel::updateTransformations(const Vec<Mat4> &transformations, int instanceCount) {
	if (instanceCount <= 0 || instanceCount > transformations.size()) {
		return;
	}

	this->instanceCount = instanceCount;

	glDeleteBuffers(1, &transformsBuffer);

	glGenBuffers(1, &transformsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, transformsBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Mat4) * instanceCount, transformations.data(), GL_STATIC_DRAW);

	auto &meshes = model == nullptr ? this->meshes : model->*(&InstancedModel::meshes);

	const int vec4Sz = sizeof(Vec4);
	const int startAttrIdx = 3;
	const int mat4ColCnt = sizeof(Mat4) / vec4Sz;
	for (int i = 0; i < meshes.size(); ++i) {
		auto &mesh = meshes[i];
		glBindVertexArray(mesh.getHandle());

		for (int j = startAttrIdx; j < startAttrIdx + mat4ColCnt; ++j) {
			glEnableVertexAttribArray(j);
			const int offsetIdx = (j - startAttrIdx) * vec4Sz;
			glVertexAttribPointer(j, 4, GL_FLOAT, false, 4 * vec4Sz, (void *)offsetIdx);
			glVertexAttribDivisor(j, 1);
		}

		glBindVertexArray(0);
	}
}

void InstancedModel::draw(Shader &shader) const {
	if (instanceCount == 0) {
		return;
	}

	auto meshes = model == nullptr ? this->meshes : model->*(&InstancedModel::meshes);
	
	for (int i = 0; i < meshes.size(); ++i) {
		for (int j = 0; j < MaterialField::MF_TEXTURES_CNT; ++j) {
			glActiveTexture(GL_TEXTURE0 + j); // set current active texture unit
			shader.setInt(meshes[i].material.getFieldName(j), j); // set ith sampler to correspond to ith texture unit
			glBindTexture(GL_TEXTURE_2D, meshes[i].material.getIntField(j).get());
		}
		shader.setField(meshes[i].material, MF_SHININESS, materialField2Type[MF_SHININESS]);

		glBindVertexArray(meshes[i].getHandle());
		glDrawElementsInstanced(GL_TRIANGLES, meshes[i].indices.size(), GL_UNSIGNED_INT, 0, instanceCount);
		glBindVertexArray(0);
	}
}
