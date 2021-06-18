#pragma once

#include "common_defines.h"
#include "common_headers.h"

struct Shader;

struct FrameBuffer {
	FrameBuffer();
	~FrameBuffer();

	bool init(int width, int height, int sampleDepth);
	void deinit();
	void use() const;
	void drawScreen(Shader &shader) const;

	bool isSamplingDepth() const {
		return sampleDepth;
	}

private:
	bool setupCustomFramebuffer();
	void setupScreenVAO();

private:
	Handle FBO;
	Handle screenVAO, screenVBO;
	Handle depthRBO; 
	Handle colorTexture;
	Handle depthTexture;

	// If MSAA is enabled we would need an intermediate FBO for post processing(pp)
	Handle ppFBO, ppColorTexure, ppDepthTexture;
	int sampleDepth;
	int width, height;
};