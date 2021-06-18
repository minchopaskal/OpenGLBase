#pragma once

// C++ std
#include <chrono>

#include "common_headers.h"
#include "common_defines.h"

#include "camera.h"
#include "cubemap.h"
#include "framebuffer.h"
#include "light.h"
#include "material.h"
#include "model.h"
#include "shader.h"
#include "uniform_buffer.h"

struct UIEngine;
struct Mesh;
using std::chrono::high_resolution_clock;

struct OpenGLEngine {
private:
	Camera camera = { glm::vec3(0.f, 0.f, 5.f) };
	GLFWwindow *window;

	FrameBuffer framebuffer;
	UniformBuffer projectionViewBuffer;
	UniformBuffer fragLightBuffer;

	CubeMap skybox;
	Model planet, rock;
	InstancedModel asteroidField;

	Model backpack;
	Model plane;
	Model cube, grassQuad, windowQuad;
	InstancedModel cubes;
	Vec<Light*> lights;

	unsigned int windowWidth = 1280, windowHeight = 768;
	struct {
		float x, y;
	} mousePos = { 640.f, 384.f };

	high_resolution_clock::time_point lastFrame{};
	high_resolution_clock::time_point lastFrameTimeCheck{};
	float deltaTime;
	float frameTime = -1.f;

	Shader shader, lightObjShader, singleColor, screenShader, skyboxShader, normalsShader;
	Shader instanceShader;
	struct {
		bool firstMove : 1;
		bool LCtrlDown : 1;
		bool LShiftDown : 1;
		bool RShiftDown : 1;
		bool keyCDown : 1;
		bool keyGDown : 1;
		bool keyHDown : 1;
		bool mouseLButtonDown : 1;
		bool mouseMiddleDown : 1;
		bool mouseRButtonDown : 1;
		bool mouse4ButtonDown : 1;
		bool mouse5ButtonDown : 1;
		bool cursorVisible : 1;
		bool grayscale : 1;
		bool explode : 1;
	} flags = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	float speedMultiplier = 1.f;

	GLubyte pixel[3];
	int pixelColor[3] = { -1, -1, -1 };

	int samples = -1; // Samples for MSAA. <=1 if it is not enabled

public:
	bool running() const;

	GLFWwindow* getGLFWwindow() const;

	void timeIt();
	void processInput();

	void render();
	void cleanup();

	// Samples must be <32 and a power of 2, else it will be ignored
	void setMultisampled(int samples);
	int isMultisampled() const;

private:
	void drawScene();

	bool init();
	void shutdown();

	bool setupGLFW();
	void setupShaders();
	void setupSkybox();
	void setupModels();
	void setupLights();
	void clearErrors();

	// GLFW callbacks
	static void frame_buf_size_callback(GLFWwindow *window, int w, int h);
	static void mouse_pos_callback(GLFWwindow*, double xPos, double yPos);
	static void scroll_callback(GLFWwindow*, double, double);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_button_callback(GLFWwindow*, int button, int action, int mods);
	static void window_size_callback(GLFWwindow* window, int width, int height);

	friend OpenGLEngine* OpenGLInit();
	friend void OpenGLShutDown();
	friend struct UIEngine;
};

OpenGLEngine* OpenGLInit();
void OpenGLShutDown();