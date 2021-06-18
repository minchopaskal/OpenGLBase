#include "framebuffer.h"

#include <stdio.h>

#include "common_defines.h"
#include "opengl_engine.h"
#include "shader.h"

FrameBuffer::FrameBuffer() : 
	FBO(-1),
	screenVAO(-1),
	screenVBO(-1),
	depthRBO(-1), 
	colorTexture(-1), 
	depthTexture(-1), 
	sampleDepth(false),
	width(0),
	height(0) {

}

FrameBuffer::~FrameBuffer() {
	deinit();
}

bool FrameBuffer::init(int width, int height, int sampleDepth) {
	deinit();
	this->sampleDepth = sampleDepth;
	this->width = width;
	this->height = height;

	if (!setupCustomFramebuffer()) {
		return false;
	}

	setupScreenVAO();
	return true;
}

void FrameBuffer::deinit() {
	glDeleteFramebuffers(1, &FBO);
	glDeleteTextures(1, &colorTexture);
	glDeleteRenderbuffers(1, &depthRBO);
	glDeleteTextures(1, &depthTexture);
	glDeleteVertexArrays(1, &screenVAO);
	glDeleteBuffers(1, &screenVBO);

	FBO = colorTexture = depthRBO = depthTexture = screenVAO = screenVBO = -1;
	sampleDepth = false;
	width = height = 0;
}

void FrameBuffer::use() const {
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void FrameBuffer::drawScreen(Shader &shader) const {
	extern OpenGLEngine *opengl;
	int samples = opengl->isMultisampled();

	if (samples > 1) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ppFBO);
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(1.f, 0.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	glBindVertexArray(screenVAO);

	glActiveTexture(GL_TEXTURE0);
	shader.setInt("colorBuffer", 0);
	glBindTexture(GL_TEXTURE_2D, samples > 1 ? ppColorTexure : colorTexture);

	if (sampleDepth) {
		glActiveTexture(GL_TEXTURE1);
		shader.setInt("depthStencilBuffer", 1);
		glBindTexture(GL_TEXTURE_2D, samples > 1 ? ppDepthTexture : depthTexture);
	}

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glEnable(GL_DEPTH_TEST);
}

void setupNormalTexture(int width, int height, int texID) {
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void setupMultisampledTexture(int width, int height, int texID, int samples) {
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texID);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, true /*fixed sample locations*/);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

bool FrameBuffer::setupCustomFramebuffer() {
	extern OpenGLEngine *opengl;

	int samples = opengl->isMultisampled();
	assert((samples <= 1 || ((samples & (samples - 1)) == 0)) && "Samples must be a power of 2!");

	// Generate the frame buffer
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &colorTexture);
	if (samples <= 1) {
		setupNormalTexture(width, height, colorTexture);
	} else {
		setupMultisampledTexture(width, height, colorTexture, samples);
	}
	GLuint framebufferTextureTarget = samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebufferTextureTarget, colorTexture, 0);

	if (sampleDepth) {
		glGenTextures(1, &depthTexture);
		if (samples <= 1) {
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBindTexture(GL_TEXTURE_2D, 0);
		} else {
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTexture);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH24_STENCIL8, width, height, true);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, framebufferTextureTarget, depthTexture, 0);
	} else {
		glGenRenderbuffers(1, &depthRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
		if (samples <= 1) {
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		} else {
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
		}

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRBO);
	}

	bool result = true;
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("ERROR::FRAMEBUFFER: Frame buffer is not complete! Status: %x\n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		result = false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (!result || samples <= 1) {
		return result;
	}

	// Generate framebuffer for post-processing
	glGenFramebuffers(1, &ppFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ppFBO);

	glGenTextures(1, &ppColorTexure);
	setupNormalTexture(width, height, ppColorTexure);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ppColorTexure, 0);
	
	if (sampleDepth) {
		glGenTextures(1, &ppDepthTexture);
		glBindTexture(GL_TEXTURE_2D, ppDepthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, ppDepthTexture, 0);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("ERROR::FRAMEBUFFER: Post-Processing frame buffer is not complete! Status: %x\n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		result = false;
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return result;
}

void FrameBuffer::setupScreenVAO() {
	const float quadVertices[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	glGenVertexArrays(1, &screenVAO);
	glGenBuffers(1, &screenVBO);

	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 4 * sizeof(float), (void *)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * sizeof(float), (void *)(2 * sizeof(float)));

	glBindVertexArray(0);
}