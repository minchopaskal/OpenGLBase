// C std
#include <math.h>

// C++ std
#include <algorithm>
#include <chrono>
#include <memory>
#include <vector>

// OpenGL
#include "opengl_engine.h"

// ImGUI
#include "ui_engine.h"

void mainLoop();
extern OpenGLEngine *opengl;
extern UIEngine *ui;

int main(int argc, char **argv) {
	OpenGLInit();
	UIInit(opengl->getGLFWwindow());

	mainLoop();

	UIShutDown();
	OpenGLShutDown();
	
	return 0;
}

void mainLoop() {
	while (opengl->running()) {
		opengl->timeIt();
		opengl->processInput();
		ui->drawUI();

		ui->render();
		opengl->render(ui);
		ui->renderDrawData();

		opengl->cleanup();
	}
}