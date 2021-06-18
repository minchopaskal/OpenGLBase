#include "ui_engine.h"
#include "opengl_engine.h"
#include "mesh.h"

UIEngine *ui = nullptr;
UIEngine* UIInit(GLFWwindow *window) {
	if (!ui) {
		ui = new UIEngine{};
		ui->init(window);
	}

	return ui;
}

void UIShutDown() {
	if (!ui) {
		return;
	}

	ui->shutdown();
	delete ui;
}

void UIEngine::init(GLFWwindow *window) {
	// Prepare ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
}

void UIEngine::shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

UIEngine* getDefaultUIEngine() {
	return nullptr;
}

void UIEngine::drawUI() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	drawMainWindow();

	if (showKeyCtrls) {
		drawControls();
	}
}

void UIEngine::render() {
	ImGui::Render();
}

void UIEngine::renderDrawData() {
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

glm::vec3 UIEngine::backgroundColor() const {
	return bgColor;
}

bool UIEngine::showControls() const {
	return showKeyCtrls;
}

bool UIEngine::drawColorEdit(const String &label, glm::vec3 &arr) {
	ImGui::ColorEdit3(label.c_str(), (float*)&arr, 1);
	bool active = ImGui::IsItemActive();
	ImGui::Separator();
	return active;
}

bool UIEngine::drawRadioGroup(const Vec<String> &labels, bool *vals) {
	bool result = false;
	for (unsigned int i = 0; i < labels.size(); ++i) {
		if (ImGui::RadioButton(labels[i].c_str(), vals[i])) {
			for (unsigned int j = 0; j < labels.size(); ++j) {
				vals[j] = (j == i);
			}
			result = true;
		}
	}
	ImGui::Separator();

	return result;
}

void UIEngine::drawMainWindow() {
	extern OpenGLEngine *opengl; // Spaghetti code, but okay for this project(..for now)

	ImGui::Begin("Options", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	bool windowHovered = ImGui::IsWindowHovered();

	ImGui::Checkbox("Show keybord controls", &showKeyCtrls);
	ImGui::Separator();

	bool sliderActive = drawColorEdit("Background color", bgColor);

	if (ImGui::Button("Subdivide")) {
		opengl->mesh->subdivide();
	}

	static unsigned mode = GL_FILL;
	if (ImGui::Button("Switch draw mode")) {
		mode = (mode == GL_LINE) ? GL_FILL : GL_LINE;
		glPolygonMode(GL_FRONT, mode);
	}
	
	ImGui::Separator();
	ImGui::Text("Mesh rotation");

	float slider;
	ImGui::SliderFloat("RotateX", &opengl->rotationX, 0.f, 360.f);
	sliderActive |= ImGui::IsItemActive();
	ImGui::SliderFloat("RotateY", &opengl->rotationY, 0.f, 360.f);
	sliderActive |= ImGui::IsItemActive();
	ImGui::SliderFloat("RotateZ", &opengl->rotationZ, 0.f, 360.f);
	sliderActive |= ImGui::IsItemActive();
	
	opengl->flags.disableOrbit = windowHovered || sliderActive;

	ImGui::Separator();

	ImGui::End();
}

void UIEngine::drawControls() {
	extern OpenGLEngine *opengl;
	
	ImGui::Begin("Keyboard controls", &showKeyCtrls,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
	opengl->flags.disableOrbit = ImGui::IsWindowHovered();

	ImGui::Text("[Esc] Exit the program.");
	ImGui::Text("[S] Subdivide the mesh.");

	ImGui::End();
}
