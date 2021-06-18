#pragma once

// C++ std
#include <chrono>

#include "common_headers.h"
#include "common_defines.h"

#include "camera.h"
#include "shader.h"

struct UIEngine;
struct Mesh;
using std::chrono::high_resolution_clock;

struct OpenGLEngine {
private:
	Camera camera;
	GLFWwindow *window;
	Mesh *mesh;

	unsigned int WIDTH = 1280, HEIGHT = 768;
	struct {
		float x, y;
	} mousePos = { 640.f, 384.f };
	float zoom;
	
	high_resolution_clock::time_point lastFrame{};
	float deltaTime;

	float rotationX = 0.f;
	float rotationY = 0.f;
	float rotationZ = 0.f;

	Shader program;
	unsigned int VAO;
	unsigned int VBO;
	unsigned int IBO;

	struct {
		bool firstMove : 1;
		bool LCtrlDown : 1;
		bool LShiftDown : 1;
		bool mouseLButtonDown : 1;
		bool mouseMiddleDown : 1;
		bool mouseRButtonDown : 1;
		bool mouse4ButtonDown : 1;
		bool mouse5ButtonDown : 1;
		bool keySDown : 1;
		bool disableOrbit : 1;
	} flags = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

public:
	bool running() const;

	GLFWwindow* getGLFWwindow() const;

	void timeIt();
	void processInput();

	void render();
	void cleanup();

private:
	void init();
	void shutdown();
	
	void initCamera();
	void prepareData();

	void clearErrors();
	void renderData();

	// GLFW callbacks
	static void frame_buf_size_callback(GLFWwindow *window, int w, int h);
	static void mouse_pos_callback(GLFWwindow*, double xPos, double yPos);
	static void scroll_callback(GLFWwindow*, double, double);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_button_callback(GLFWwindow*, int button, int action, int mods);
	static void window_size_callback(GLFWwindow* window, int width, int height);

	friend UIEngine;
	friend OpenGLEngine* OpenGLInit();
	friend void OpenGLShutDown();
};

OpenGLEngine* OpenGLInit();
void OpenGLShutDown();