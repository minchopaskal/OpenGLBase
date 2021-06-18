#pragma once

#include "common_defines.h"
#include "drawable.h"

struct CubeMap : DrawableInterface {
	CubeMap();
	~CubeMap();

	void init(const Vec<String> &maps);

	void bindTextureToShader(Shader &shader, int idx) const;

	void draw(Shader &shader) const override;

private:
	void loadCubemap(const Vec<String> &maps);

private:
	Handle VAO, VBO;
	Handle texID;
};