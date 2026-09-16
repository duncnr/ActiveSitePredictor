#include <stdio.h>
#include <glfw/glfw3.h>

struct Window {
	GLFWwindow* window;

	void loop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	// constructor
	Window() {
		if (glfwPlatformSupported(GLFW_PLATFORM_WAYLAND)) {
			glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
		}

		if (!glfwInit()) {
			fprintf(stderr, "GLFW Failed to Initialize\n");
			return;
		}

		GLFWwindow* window = glfwCreateWindow(800, 600, "Hello World!", NULL, NULL);
		if (!window) {
			glfwTerminate();
			fprintf(stderr, "GLFW Window Failed to Initialize\n");
			return;
		}

		glfwMakeContextCurrent(window);
	}

	// destructor
	~Window() {
		glfwTerminate();
	}
};
