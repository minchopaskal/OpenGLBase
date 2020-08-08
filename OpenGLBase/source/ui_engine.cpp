#include "ui_engine.h"

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

void UIEngine::drawColorEdit(const String &label, glm::vec3 &arr) {
	ImGui::ColorEdit3(label.c_str(), (float*)&arr, 1);
	ImGui::Separator();
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
	ImGui::Begin("Options", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Show keybord controls", &showKeyCtrls);
	ImGui::Separator();

	drawColorEdit("Background color", bgColor);

	if (ImGui::Button("Button")) {
		
	}

	ImGui::Text("This is a text.");
	ImGui::Separator();

	bool cb;
	ImGui::Checkbox("CheckBox", &cb);
	ImGui::Separator();

	float slider;
	ImGui::SliderFloat("Slider", &slider, 0.1f, 10.f);
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Slider currently not working...");
	}
	ImGui::Separator();

	ImGui::End();
}

void UIEngine::drawControls() {
	ImGui::Begin("Keyboard controls", &showKeyCtrls,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
	
	ImGui::Text("[Esc] Exit the program.");

	ImGui::End();
}
