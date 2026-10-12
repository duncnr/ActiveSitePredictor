#pragma once

#include <glfw/glfw3.h>
#include <VkBootstrap.h>

struct Window {
	GLFWwindow* window;
	vkb::Instance vkb_instance;
	VkSurfaceKHR vk_surface; // connection handle between vulkan and window
	vkb::Device vkb_device; // logical device referencing actual hardware
	VkQueue vk_graphics_queue;

	// Note that the swapchain format should be in BGRA format (as opposed to RGBA)
	// This is because most GPUs have a display controller which uses a little endian format
	// (It is also required on windows)
	// If we store the swapchain images as RGBA, a hidden conversion is required.
	// NOTE THAT we should NOT be rendering directly to the swapchain
	// We should be rendering to an FP16 (RBGA_F16) texture instead, then clamping to the framebuffer
	// (Gamma correction for example, should be a postprocess step and not done within the swapchain)
	vkb::Swapchain vkb_swapchain;

	void update();
	bool init_vulkan();
	bool should_close();

	Window();
	~Window();
};
