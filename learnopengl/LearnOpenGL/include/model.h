#pragma once

#include "common_defines.h"
#include "drawable.h"
#include "mesh.h"
#include "shader.h"


struct aiMaterial;
struct aiMesh;
struct aiNode;
struct aiScene;
struct Shader;

enum aiTextureType;

struct Model : DrawableInterface {
public:
	Model() {}
	void init(const String &path) {
		loadModel(path);
	}
	void deinit();
	virtual ~Model();

	void draw(Shader &shader) const override;
protected:
	// model data
	Vec<Mesh> meshes;
	
private:
	Map<size_t, Texture> textureHashes;
	String directory;

	void loadModel(const String &path);
	void processNode(aiNode *node, const aiScene *scene);
	void processMesh(aiMesh *mesh, const aiScene *scene);
	void loadMaterialTexturesForMesh(Material &material, aiMaterial *mat, aiTextureType type);
};

struct InstanceUpdateParams {
	Vec3 position;
	Vec3 scale;
};

// TODO: Add pivot so outlined object's center is the same as the actual object's center
struct Instance : DrawableInterface {
private:
	friend void instanceUpdater(DrawableInterface *, UpdateParams);

public:
	Instance();

	void init(Model *model, bool outlined = false, Shader *outlineShader = nullptr);
	void draw(Shader &shader) const override;

private:
	Model *model;
	Shader *outlineShader;
	Vec3 position;
	Vec3 scale;
	bool inited;
	bool outlined;
};

struct InstancedModel : Model {
	InstancedModel();

	void init(const String &modelPath, const Vec<Mat4> &transformations, int instanceCount);
	void init(Model *model, const Vec<Mat4> &transformations, int instanceCount);

	void deinit();
	void updateTransformations(const Vec<Mat4> &transformations, int instanceCount);

	void draw(Shader &shader) const override;

private:
	Model *model; // In case we use an already loaded model
	Handle transformsBuffer;
	int instanceCount;
};