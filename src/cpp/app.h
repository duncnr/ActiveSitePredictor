#pragma once

#include "glm/ext/scalar_constants.hpp"
#include "window.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>

inline glm::vec3 euler_2_dir(glm::vec2 rotation, float extrude) {
    glm::vec3 dir;
    dir.x = cos(rotation.x) * cos(rotation.y);
    dir.y = cos(rotation.x) * sin(rotation.y);
    dir.z = sin(rotation.x);
    return glm::normalize(dir) * extrude;
}

// Holds everything together
struct App {
	Window* window = nullptr;
	std::vector<glm::vec4> points = {};
	glm::vec2 rotation = {};
	glm::vec2 mouse_delta = {};
	float extrusion = 10;
	bool init_glfw() {
		//if (glfwPlatformSupported(GLFW_PLATFORM_WAYLAND)) {
		//	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
		//}

		return glfwInit();
	}

	void loop() {
		std::vector<glm::vec4> points2 = {};
		for (int z = -10; z < 10; z++) {
			for (int y = -10; y < 10; y++) {
				for (int x = -10; x < 10; x++) {
					points2.push_back(glm::vec4{x * 3, y * 3, z * 3, 1.0f});
				}
			}
		}
		while (window && !window->should_close()) {
			static float frametime = 0.0;
			window->camera_pos = euler_2_dir(rotation, extrusion);
			rotation += mouse_delta * frametime;
			rotation.x = glm::clamp(rotation.x, glm::pi<float>() / -2.1f, glm::pi<float>() / 2.1f);
			mouse_delta *= glm::clamp(1.0f - frametime, 0.0f, 1.0f);

			frametime = static_cast<float>(window->draw(points2));
			printf("frametime (ms): %f\n", frametime * 1000);
		}
	}

	App() {
		if (!init_glfw()) {
			fprintf(stderr, "GLFW Failed to Initialize\n");
			return;
		}

		window = new Window();

		static float& pitch = mouse_delta.x;
		static float& yaw = mouse_delta.y;
		glfwSetCursorPosCallback(window->window, [](GLFWwindow* window, double delta_x, double delta_y) {
			yaw += delta_x * 0.01;
			pitch += delta_y * 0.01;

			glfwSetCursorPos(window, 0, 0);
		});

		static float& extrusion = this->extrusion;
		glfwSetScrollCallback(window->window, [](GLFWwindow* window, double delta_x, double delta_y) {
			extrusion = glm::max(static_cast<float>(extrusion - delta_y), 1.0f);
		});
	}

	~App() {
		delete window;

		glfwTerminate();
	}
};
