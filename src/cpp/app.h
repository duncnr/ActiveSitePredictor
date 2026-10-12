#pragma once

#include "window.h"

// Holds everything together
struct App {
	Window* window = nullptr;
	bool init_glfw() {
		//if (glfwPlatformSupported(GLFW_PLATFORM_WAYLAND)) {
		//	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
		//}

		return glfwInit();
	}

	App() {
		if (!init_glfw()) {
			fprintf(stderr, "GLFW Failed to Initialize\n");
			return;
		}

		window = new Window();
	}

	~App() {
		delete window;

		glfwTerminate();
	}
};
