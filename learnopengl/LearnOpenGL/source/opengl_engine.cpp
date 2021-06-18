#include "opengl_engine.h"

// C std
#include <cmath>

// C++ std
#include <algorithm>

// User
#include "ui_engine.h"
#include "mesh.h"

template <class T>
unsigned int sizeOf(const Vec<T> &v) {
	return (unsigned int)v.size() * sizeof(T);
}

template <class T>
void *dataOf(const Vec<T> &v) {
	return (void *)v.data();
}

template <class T>
unsigned int countOf(const Vec<T> &v) {
	return (unsigned int)v.size();
}

OpenGLEngine *opengl = nullptr;
OpenGLEngine *OpenGLInit() {
	if (!opengl) {
		opengl = new OpenGLEngine{};
		if (!opengl->init()) {
			fprintf(stderr, "ERROR while initialized OpenGL!\n");
			exit(0xBEBE);
		}
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

bool OpenGLEngine::init() {
	if (!setupGLFW()) {
		return false;
	}

	srand((unsigned int)glfwGetTime());

	setMultisampled(32);

	if (!framebuffer.init(windowWidth, windowHeight, true /* sampleDepth */)) {
		glfwTerminate();
		return false;
	}

	projectionViewBuffer.init(2 * sizeof(Mat4));
	fragLightBuffer.init(sizeof(Vec3) + 2 * sizeof(bool));

	setupSkybox();
	setupShaders();
	setupModels();
	setupLights();

	return true;
}

void OpenGLEngine::shutdown() {
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

	float checkTime = duration_cast<duration<float, std::milli>>(currentTime - lastFrameTimeCheck).count();
	if (frameTime < 0.f || checkTime > 100.f) {
		frameTime = duration_cast<duration<float, std::milli>>(currentTime - lastFrame).count();
		lastFrameTimeCheck = currentTime;
	}

	lastFrame = currentTime;
}

void OpenGLEngine::processInput() {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (flags.firstMove) {
		return;
	}

	const float speed = speedMultiplier * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, speed);

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, speed);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, speed);

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, speed);

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, speed);

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, speed);

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.ProcessMouseMovement(speed, 0);

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera.ProcessMouseMovement(speed, 0);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.ProcessMouseMovement(0, speed);

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.ProcessMouseMovement(0, speed);
}

bool OpenGLEngine::setupGLFW() {
	glfwInit();

	window = glfwCreateWindow(windowWidth, windowHeight, "Test", 0, 0);
	if (window == nullptr) {
		fprintf(stderr, "GLFW createWindow ERROR!\n");
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "GLAD LoadGLLoader ERROR!\n");
		glfwTerminate();
		return false;
	}

	glViewport(0, 0, windowWidth, windowHeight);
	glfwSetFramebufferSizeCallback(window, frame_buf_size_callback);
	glfwSetCursorPosCallback(window, mouse_pos_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	return true;
}

void OpenGLEngine::setupShaders() {
	shader.init("res\\shaders\\vert_light.glsl", "res\\shaders\\geom_explode.glsl", "res\\shaders\\frag_light.glsl");
	normalsShader.init("res\\shaders\\vert_std.glsl", "res\\shaders\\geom_normals.glsl", "res\\shaders\\frag_single_color.glsl");
	lightObjShader.init("res\\shaders\\vert_light.glsl", nullptr, "res\\shaders\\frag_lightobj.glsl");
	singleColor.init("res\\shaders\\vert_std.glsl", nullptr, "res\\shaders\\frag_single_color.glsl");
	screenShader.init("res\\shaders\\vert_screen.glsl", nullptr, "res\\shaders\\frag_screen.glsl");
	skyboxShader.init("res\\shaders\\vert_skybox.glsl", nullptr, "res\\shaders\\frag_skybox.glsl");
	instanceShader.init("res\\shaders\\vert_instanced.glsl", "res\\shaders\\geom_explode.glsl", "res\\shaders\\frag_light.glsl");
}

void OpenGLEngine::setupSkybox() {
	const Vec<String> maps = {
		"res\\imgs\\skybox\\right.jpg",
		"res\\imgs\\skybox\\left.jpg",
		"res\\imgs\\skybox\\top.jpg",
		"res\\imgs\\skybox\\bottom.jpg",
		"res\\imgs\\skybox\\front.jpg",
		"res\\imgs\\skybox\\back.jpg"
	};

	skybox.init(maps);
}

void OpenGLEngine::setupModels() {
	cube.init("res\\models\\cube\\cube.obj");
	plane.init("res\\models\\plane\\plane.obj");
	grassQuad.init("res\\models\\grass\\grass.obj");
	windowQuad.init("res\\models\\window\\window.obj");
	plane.init("res\\models\\plane\\plane.obj");

	int instanceCount = 3;
	Vec<Mat4> transforms;
	transforms.resize(instanceCount);

	for (int i = 0; i < instanceCount; ++i) {
		Mat4 &mat = transforms[i];

		mat = Mat4(1.f);
		float dist = rand() % 3 + 1;
		mat = glm::scale(mat, Vec3(dist));
		mat = glm::translate(mat, Vec3((i + 1) * 5, 0.f, 0.f));
	}
	cubes.init(&cube, transforms, instanceCount);

	/*backpack.init("res\\models\\backpack\\backpack.obj");
	planet.init(R"(res\models\planet\planet.obj)");
	rock.init("res\\models\\rock\\rock.obj");
	
	instanceCount = 10000;
	transforms.clear();
	transforms.resize(instanceCount);
	for (int i = 0; i < instanceCount; ++i) {
		auto &mat = transforms[i];
		mat = Mat4(1.f);

		float angle = float(i) / float(instanceCount) * 360;
		float y = 2 * (float)rand() / RAND_MAX - 1.f;
		
		float displecement = (rand() % 500) / 100.f - 2.5f;
		float x = glm::cos(glm::radians(angle)) * 20.f - displecement;
		displecement = (rand() % 500) / 100.f - 2.5f;
		float z = glm::sin(glm::radians(angle)) * 20.f - displecement;
		mat = glm::translate(mat, Vec3(x, y, z));

		float scale = (float)rand() / RAND_MAX;
		scale *= 0.7;
		scale += 0.1;
		scale *= 0.1;
		mat = glm::scale(mat, Vec3(scale));

		float rotate = (float)rand();
		mat = glm::rotate(mat, glm::radians(rotate), Vec3(0.2f, 0.4f, 0.6f));
	}

	asteroidField.init(&rock, transforms, instanceCount);*/
}

void updateLightPos(DrawableInterface *obj, void *p) {
	PointLight *l = dynamic_cast<PointLight*>(obj);
	auto &pos = l->position;

	float angle = 100 * glm::radians(float(glfwGetTime()));
	pos.x = 2 * glm::sin(angle);
	pos.z = 2 * glm::cos(angle);
}

void OpenGLEngine::setupLights() {
	// Prepare lights
	const int cntLights = 4;
	const int cntPointLights = 2;
	lights.resize(cntLights);

	Vec3 lightColor(1.f);
	Vec3 lightDiffuse = lightColor;
	Vec3 lightAmbient = /*lightDiffuse **/ Vec3(0.2f);
	Vec3 lightSpecular(1.f);

	Vec3 dirLightDirection = Vec3(-1.f, 0.f, 1.f);

	Vec3 pointLightPositions[cntPointLights] = {
		Vec3(1.f, 0.f, -2.f),
		Vec3(0.f, 0.f, 2.f),
	};
	Vec3 attenuation = Vec3(1.f, 0.022f, 0.0019f);

	float spotLightCuttoff = glm::cos(glm::radians(15.f));

	lights[0] = new DirectionalLight("dirLight", lightAmbient, lightDiffuse, lightSpecular, dirLightDirection);
	for (int i = 0; i < cntPointLights; ++i) {
		char name[16];
		sprintf(name, "lights[%d]", i);
		lights[1 + i] = new PointLight(
			name,
			&cube,
			lightAmbient,
			lightDiffuse,
			lightSpecular,
			pointLightPositions[i],
			attenuation
		);

		if (i == 1) {
			lights[1 + i]->f = updateLightPos;
		}
	}
	lights[3] = new SpotLight(
		"spotLight",
		camera,
		lightAmbient,
		lightDiffuse,
		lightSpecular,
		attenuation,
		spotLightCuttoff
	);
}

void OpenGLEngine::render() {
	framebuffer.use();
	drawScene();

	screenShader.use();
	screenShader.setBool("grayscale", flags.grayscale);
	framebuffer.drawScreen(screenShader);
}

void updateLights(Vec<Light *> &lights) {
	for (int i = 0; i < lights.size(); ++i) {
		lights[i]->update(nullptr);
	}
}

void setupLightsForShader(Vec<Light*> &lights, Shader &shader) {
	for (int i = 0; i < lights.size(); ++i) {
		lights[i]->setFields(shader);
	}
}

void OpenGLEngine::drawScene() {
	extern UIEngine *ui;

	auto bgColor = ui->backgroundColor();
	glClearColor(bgColor[0], bgColor[1], bgColor[2], 1.f);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0x00);

	glEnable(GL_CULL_FACE);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	auto projection = glm::perspective(glm::radians(camera.FOV()), windowWidth / float(windowHeight), 0.01f, 1000.f);
	auto view = camera.GetViewMatrix();

	updateLights(lights);

	// Setup common uniforms of shaders
	projectionViewBuffer.subData(glm::value_ptr(projection), sizeof(Mat4));
	projectionViewBuffer.subData(glm::value_ptr(view), sizeof(Mat4));
	projectionViewBuffer.bind(0);

	const bool notTrue = false;
	fragLightBuffer.subData(glm::value_ptr(camera.Position), sizeof(Vec3));
	fragLightBuffer.subData(&notTrue, sizeof(bool));
	fragLightBuffer.subData(&notTrue, sizeof(bool));
	fragLightBuffer.bind(1);

	singleColor.use();
	skyboxShader.use();
	skyboxShader.setMat4("VP", projection * Mat4(Mat3(view)));

	normalsShader.use();
	normalsShader.setMat4("proj", projection);

	const int skyboxSampler = MF_TEXTURES_CNT; 
	shader.use();
	if (flags.explode) {
		shader.setFloat("explodeMagnitude", 2 * ((glm::sin(glfwGetTime()) + 1.f) / 2.f));
	}
	skybox.bindTextureToShader(shader, MF_TEXTURES_CNT);
	setupLightsForShader(lights, shader);
	
	instanceShader.use();
	skybox.bindTextureToShader(instanceShader, MF_TEXTURES_CNT);
	setupLightsForShader(lights, instanceShader);

	//auto mat = Mat4(1.f);
	//shader.setMat4("modelMat", mat);
	//shader.setMat4("normalMat", glm::transpose(glm::inverse(mat)));
	//planet.draw(shader);

	//instanceShader.use();
	//instanceShader.setBool("explode", flags.explode);
	//if (flags.explode) {
	//	instanceShader.setFloat("explodeMagnitude", 1.5f * ((glm::sin(glfwGetTime()) + 1.f) / 2.f));
	//}
	//asteroidField.draw(instanceShader);
	//instanceShader.setBool("explode", false);

	// draw opaque normal objects
	shader.use();
	glDisable(GL_CULL_FACE); // plane doesn't need face culling
	Instance planeInstance;
	planeInstance.init(&plane);
	InstanceUpdateParams planeParams = { Vec3(-50.f, -0.001f, -50.f), Vec3(100.f) };
	planeInstance.update(&planeParams);
	planeInstance.draw(shader);
	glEnable(GL_CULL_FACE);

	Instance cubeInstance;
	cubeInstance.init(&cube);
	InstanceUpdateParams iup[] = {
		{Vec3(0.f), Vec3(1.f)},
		{Vec3(2.f, 0.f, 2.f), Vec3(1.f)}
	};
	// TODO: try dynamic env mapping!
	for (int i = 0; i < 2; ++i) {
		cubeInstance.update(&iup[i]);
		cubeInstance.draw(shader);
	}
	
	lightObjShader.use();
	lightObjShader.setVec3("lightColor", Vec3(1.f));
	auto modelMat = Mat4(1.f);
	for (int i = 0; i < lights.size(); ++i) {
		auto maybePos = lights[i]->getVec3Field(LF_POSITION);
		auto maybeSpotLight = lights[i]->getFloatField(LF_CUTOFF);
		if (!maybePos || maybeSpotLight) {
			continue;
		}
		modelMat = Mat4(1.f);
		modelMat = glm::translate(modelMat, maybePos.get());
		modelMat = glm::scale(modelMat, Vec3(.2f));
		lightObjShader.setMat4("modelMat", modelMat);

		lights[i]->draw(lightObjShader);
	}

	instanceShader.use();
	instanceShader.setBool("explode", flags.explode);
	if (flags.explode) {
		instanceShader.setFloat("explodeMagnitude", 1.5f * ((glm::sin(glfwGetTime()) + 1.f) / 2.f));
	}
	cubes.update(nullptr);
	cubes.draw(instanceShader);
	instanceShader.setBool("explode", false);

	// outlined objects
	//Instance backpackInstance;
	//backpackInstance.init(&backpack/*, true, &singleColor*/);
	//InstanceUpdateParams backpackIUP[] = {
	//	{Vec3(10.f, 0.f, 0.f), Vec3(1.4f)},
	//	{Vec3(-7.f, 5.f, 2.f), Vec3(1.f)},
	//	{Vec3(5.f, 5.f, 2.f), Vec3(.5f)},
	//};
	//shader.use();
	//shader.setBool("explode", flags.explode); // explode only the backpacks
	//if (flags.explode) {
	//	shader.setFloat("explodeMagnitude", 1.5f * ((glm::sin(glfwGetTime()) + 1.f) / 2.f));
	//}
	//for (int j = 0; j < sizeof(backpackIUP) / sizeof(InstanceUpdateParams); ++j) {
	//	backpackInstance.update(&backpackIUP[j]);
	//	backpackInstance.draw(shader);
	//}
	//shader.setBool("explode", false);

	//draw skybox
	skyboxShader.use();
	skybox.draw(skyboxShader);

	// semi-transparent objects
	shader.use();
	glDisable(GL_CULL_FACE);
	static const int wpCnt = 5;
	static Vec3 windowPositions[] = {
		Vec3(-1.5f, 0.0f, -0.48f),
		Vec3(1.5f, 0.0f, 0.51f),
		Vec3(0.0f, 0.0f, 0.7f),
		Vec3(-0.3f, 0.0f, -2.3f),
		Vec3(0.5f, 0.0f, -0.6f),
	};

	auto &cam = this->camera;
	std::sort(windowPositions, windowPositions + wpCnt,
		[cam](const Vec3 &a, const Vec3 &b) {
			float dist1 = glm::length2(cam.Position - a);
			float dist2 = glm::length2(cam.Position - b);
			return dist1 > dist2;
		}
	);

	Instance windowInstance;
	windowInstance.init(&windowQuad);
	for (int i = 0; i < wpCnt; ++i) {
		InstanceUpdateParams p{ windowPositions[i], Vec3(1.f) };
		windowInstance.update(&p);
		windowInstance.draw(shader);
	}
}

void OpenGLEngine::cleanup() {
	glfwSetInputMode(window, GLFW_CURSOR, flags.cursorVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void OpenGLEngine::setMultisampled(int samples) {
	const bool isPowerOfTwo = (samples & (samples - 1)) == 0;
	if (samples > 1 && isPowerOfTwo) {
		this->samples = samples;
	} else {
		this->samples = 1;
	}
}

int OpenGLEngine::isMultisampled() const {
	return samples;
}

void OpenGLEngine::clearErrors() {
	while (glGetError() != GL_NO_ERROR);
}

void OpenGLEngine::frame_buf_size_callback(GLFWwindow *window, int w, int h) {
	glViewport(0, 0, w, h);
}

void OpenGLEngine::mouse_pos_callback(GLFWwindow *window, double xPos, double yPos) {
	auto &flags = opengl->flags;
	auto &mousePos = opengl->mousePos;
	auto &camera = opengl->camera;

	if (flags.firstMove) {
		mousePos.x = float(xPos);
		mousePos.y = float(yPos);
		flags.firstMove = false;
	}

	float deltaX = float(xPos) - mousePos.x;
	float deltaY = mousePos.y - float(yPos);

	mousePos.x = float(xPos);
	mousePos.y = float(yPos);

	if (flags.mouseRButtonDown) {
		glReadPixels(int(mousePos.x), int(mousePos.y), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, opengl->pixel);
		opengl->pixelColor[0] = int(opengl->pixel[0]);
		opengl->pixelColor[1] = int(opengl->pixel[1]);
		opengl->pixelColor[2] = int(opengl->pixel[2]);
	}

	if (!opengl->flags.cursorVisible) {
		camera.ProcessMouseMovement(deltaX, deltaY);
	}
}

void OpenGLEngine::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto &flags = opengl->flags;
	bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);

	switch (key) {
	case GLFW_KEY_LEFT_CONTROL:
		flags.LCtrlDown = pressed;
		break;
	case GLFW_KEY_LEFT_SHIFT:
		opengl->speedMultiplier = 1.f + pressed * 4.f;
		flags.LShiftDown = pressed;
		break;
	case GLFW_KEY_RIGHT_SHIFT:
		opengl->speedMultiplier = 1.f + pressed * 4.f;
		flags.RShiftDown = pressed;
		break;
	case GLFW_KEY_C:
		if (!flags.keyCDown && pressed) {
			opengl->flags.cursorVisible = !opengl->flags.cursorVisible;
		}
		flags.keyCDown = pressed;
		break;
	case GLFW_KEY_G:
		if (!flags.keyGDown && pressed) {
			opengl->flags.grayscale = !opengl->flags.grayscale;
		}
		flags.keyGDown = pressed;
		break;
	case GLFW_KEY_H:
		if (!flags.keyHDown && pressed) {
			opengl->flags.explode = !opengl->flags.explode;
		}
		flags.keyHDown = pressed;
		break;
	}
}

void OpenGLEngine::mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
	auto &flags = opengl->flags;
	auto &mousePos = opengl->mousePos;
	bool pressed = (action == GLFW_PRESS);

	switch (button) {
	case GLFW_MOUSE_BUTTON_RIGHT:
		if (pressed) {
			glReadPixels(int(mousePos.x), int(mousePos.y), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, opengl->pixel);
			opengl->pixelColor[0] = int(opengl->pixel[0]);
			opengl->pixelColor[1] = int(opengl->pixel[1]);
			opengl->pixelColor[2] = int(opengl->pixel[2]);
		}
		if (!pressed && flags.mouseRButtonDown) {
			opengl->pixelColor[0] = -1;
			opengl->pixelColor[1] = -1;
			opengl->pixelColor[2] = -1;
		}
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
	opengl->windowWidth = width;
	opengl->windowHeight = height;
	auto &framebuffer = opengl->framebuffer;
	framebuffer.init(width, height, framebuffer.isSamplingDepth());
}

// testing scrollback
void OpenGLEngine::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	opengl->camera.ProcessMouseScroll(yoffset);
}
