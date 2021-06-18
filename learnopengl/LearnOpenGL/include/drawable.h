#pragma once

struct Shader;
struct DrawableInterface;

using UpdateParams = void*;
using UpdateFunction = void(*)(DrawableInterface*, UpdateParams);

static void noop(DrawableInterface*, void*) { }

struct DrawableInterface {
	UpdateFunction f;

	DrawableInterface() : f(noop) { }
	
	void update(UpdateParams params) { 
		f(this, params);
	};

	// Expects shader.use() to be called beforehand!
	virtual void draw(Shader &shader) const = 0;
};