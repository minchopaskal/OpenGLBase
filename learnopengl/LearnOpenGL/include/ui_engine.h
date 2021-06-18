#pragma once

#include "common_headers.h"
#include "common_defines.h"

struct OpenGLEngine;

struct UIEngine {
private:
	glm::vec3 bgColor = { .0f, .0f, .0f };
	bool showKeyCtrls = false;

public:

	void drawUI(); // must be called before this->render()
	void render(); // must be called before opengl->render()
	void renderDrawData(); // must be called after opengl->render()

	glm::vec3 backgroundColor() const;
	bool showControls() const;

private:
	void drawColorEdit(const String &label, glm::vec3 &arr);
	bool drawRadioGroup(const Vec<String> &labels, bool *vals);

	void drawMainWindow();
	void drawControls();

	void init(GLFWwindow *window);
	void shutdown();

	friend UIEngine* UIInit(GLFWwindow *window);
	friend void UIShutDown();
};

UIEngine* UIInit(GLFWwindow *window);
void UIShutDown();
