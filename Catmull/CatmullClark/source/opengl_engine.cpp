#include "opengl_engine.h"

// C std
#include <cmath>

// User
#include "ui_engine.h"
#include "mesh.h"

OpenGLEngine *opengl = nullptr;
OpenGLEngine *OpenGLInit() {
	if (!opengl) {
		opengl = new OpenGLEngine{};
		opengl->init();
	}

	return opengl;
}

void OpenGLShutDown() {
	if (!opengl) {
		return;
	}

	opengl->shutdown();
	delete opengl;
}

void OpenGLEngine::init() {
	glfwInit();
	window = glfwCreateWindow(WIDTH, HEIGHT, "Test", 0, 0);
	if (window == nullptr) {
		fprintf(stderr, "GLFW createWindow ERROR!\n");
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "GLAD LoadGLLoader ERROR!\n");
		glfwTerminate();
		exit(-1);
	}

	glViewport(0, 0, WIDTH, HEIGHT);
	glfwSetFramebufferSizeCallback(window, frame_buf_size_callback);
	glfwSetCursorPosCallback(window, mouse_pos_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glEnable(GL_DEPTH_TEST);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &IBO);

	mesh = newDefaultCube();
	prepareData();

	// Prepare shader
	program.init("shaders\\vert_std.glsl", nullptr, "shaders\\frag_std.glsl");
	program.use();
}

void OpenGLEngine::shutdown() {
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
	glDeleteVertexArrays(1, &VAO);

	deleteDefaultCube(mesh);

	glfwDestroyWindow(window);
	glfwTerminate();
}

bool OpenGLEngine::running() const {
	return !glfwWindowShouldClose(window);
}

GLFWwindow* OpenGLEngine::getGLFWwindow() const {
	return window;
}

void OpenGLEngine::timeIt() {
	using std::chrono::duration;
	using std::chrono::duration_cast;

	auto currentTime = high_resolution_clock::now();
	deltaTime = duration_cast<duration<float>>(currentTime - lastFrame).count();
	lastFrame = currentTime;
}

void OpenGLEngine::processInput() {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

void OpenGLEngine::render() {
	extern UIEngine *ui; // Somebody Toucha My Spaghet!

	auto projection = glm::perspective(camera.FOV(), WIDTH / float(HEIGHT), 0.01f, 100.f);
	auto view = glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 0.0f, -3.0f));
	auto model = glm::rotate(glm::mat4(1.f), float(glm::radians(rotationX)), glm::vec3(1.f, 0.f, 0.f));
	model = glm::rotate(model, float(glm::radians(rotationY)), glm::vec3(0.f, 1.f, 0.f));
	model = glm::rotate(model, float(glm::radians(rotationZ)), glm::vec3(0.f, 0.f, 1.f));
	model = glm::scale(model, glm::vec3(3.f, 3.f, 3.f));
	program.setMat4("MVP", projection * view * model);

	program.setVec3("color", Vec3(0.f, 0.8f, 0.5f));

	auto bgColor = ui->backgroundColor();
	glClearColor(bgColor[0], bgColor[1], bgColor[2], 1.f);
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderData();
}

void OpenGLEngine::cleanup() {
	glfwPollEvents();
	glfwSwapBuffers(window);
}

void OpenGLEngine::clearErrors() {
	while (glGetError() != GL_NO_ERROR);
}

template <class T>
unsigned int sizeOf(const Vec<T> &v) {
	return v.size() * sizeof(T);
}

template <class T>
void* dataOf(const Vec<T> &v) {
	return (void*)v.data();
}

template <class T>
unsigned int countOf(const Vec<T> &v) {
	return v.size();
}

template <class T>
unsigned int countOfElements(const Vec<T>& v) {
	return v.size() * v[0].length();
}

void OpenGLEngine::initCamera() {

}

void OpenGLEngine::prepareData() {
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeOf(mesh->points()), dataOf(mesh->points()), GL_STATIC_DRAW);
	glVertexAttribPointer(0, Mesh::TRI_FACE_VERTS, GL_FLOAT, false, Mesh::TRI_FACE_VERTS * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOf(mesh->triFaces), dataOf(mesh->triFaces), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void OpenGLEngine::renderData() {
	if (mesh->subdivided) {
		mesh->subdivided = false;
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &IBO);

		glGenBuffers(1, &VBO);
		glGenBuffers(1, &IBO);

		prepareData();
	}

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, countOfElements(mesh->triFaces), GL_UNSIGNED_INT, 0);
}

void OpenGLEngine::frame_buf_size_callback(GLFWwindow *window, int w, int h) {
	glViewport(0, 0, w, h);
}

void OpenGLEngine::mouse_pos_callback(GLFWwindow *window, double xPos, double yPos) {
	auto &flags = opengl->flags;
	auto &mousePos = opengl->mousePos;

	if (flags.firstMove) {
		mousePos.x = float(xPos);
		mousePos.y = float(yPos);
		flags.firstMove = false;
	}

	float deltaX = float(xPos) - mousePos.x;
	float deltaY = mousePos.y - float(yPos);

	if (flags.mouseLButtonDown && !flags.disableOrbit) {
		opengl->rotationX -= deltaY;
		opengl->rotationY += deltaX;
	}

	mousePos.x = float(xPos);
	mousePos.y = float(yPos);
}

void OpenGLEngine::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto &flags = opengl->flags;
	bool pressed = (action == GLFW_PRESS);

	switch (key) {
	case GLFW_KEY_LEFT_CONTROL:
		flags.LCtrlDown = pressed;
		break;
	case GLFW_KEY_LEFT_SHIFT:
		flags.LShiftDown = pressed;
		break;
	case GLFW_KEY_S:
		if (pressed && !flags.keySDown) {
			opengl->mesh->subdivide();
		}
		flags.keySDown = pressed;
		break;
	}
}

void OpenGLEngine::mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
	auto& flags = opengl->flags;
	bool pressed = (action == GLFW_PRESS);

	switch (button) {
	case GLFW_MOUSE_BUTTON_RIGHT:
		flags.mouseRButtonDown = pressed;
		break;
	case GLFW_MOUSE_BUTTON_LEFT:
		flags.mouseLButtonDown = pressed;
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		flags.mouseMiddleDown = pressed;
		break;
	case GLFW_MOUSE_BUTTON_4:
		flags.mouse4ButtonDown = pressed;
		break;
	case GLFW_MOUSE_BUTTON_5:
		flags.mouse5ButtonDown = pressed;
		break;
	default: break;
	}
}

void OpenGLEngine::window_size_callback(GLFWwindow* window, int width, int height) {
	opengl->WIDTH = width;
	opengl->HEIGHT = height;
}

// testing scrollback
void OpenGLEngine::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	const static int scrollMultiplier = 5;
	auto &zoom = opengl->zoom;

	zoom = Min(Max(0.1f, zoom + scrollMultiplier * float(yoffset) * opengl->deltaTime), 10.f);
}
