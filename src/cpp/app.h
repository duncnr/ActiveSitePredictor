#pragma once

#include "window.h"

// Holds everything together
struct App {
	Window* window = nullptr;
	std::vector<glm::vec3> points;
	bool init_glfw() {
		//if (glfwPlatformSupported(GLFW_PLATFORM_WAYLAND)) {
		//	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
		//}

		return glfwInit();
	}

	void loop() {
		std::vector<glm::vec4> points = {
			glm::vec4{1, 0, 0, 1},
			glm::vec4{0, 0, 0, 1},
			glm::vec4{0, 0, 1, 1}
		};
		while (window && !window->should_close()) {
			window->draw(points);
		}
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
