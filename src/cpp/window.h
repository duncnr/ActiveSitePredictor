#pragma once

#include <vector>
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>
#include "VkBootstrapDispatch.h"

struct Frame {
	VkFence fence_inflight;          // GPU -> CPU sync
	VkSemaphore semaphore_available; // CPU -> GPU or GPU -> GPU sync
	VkCommandBuffer command_buffer;
};

struct SwapchainImage {
	VkSemaphore semaphore_finished;
    VkImage image;
    VkImageView image_view;
    VkFramebuffer framebuffer;
};

struct Window {
	GLFWwindow* window;

	vkb::Instance vkb_instance;
	VkSurfaceKHR vk_surface; // connection handle between vulkan and window
	vkb::Device vkb_device; // logical device referencing actual hardware
	vkb::DispatchTable vkb_device_dispatch; // table which holds Vulkan device function pointers

	// most graphics cards have multiple queues, consisting of (I believe) graphics, compute, and transfer (present?)
	VkQueue vk_graphics_queue;
	VkQueue vk_present_queue;

	// command pools are pre allocated "pool"s of command buffers (which are a list of gpu commands to submit to a queue object)
	// this is an technique which more efficiency manages GPU   memory
	VkCommandPool vk_command_pool;

	// Note that the swapchain format should be in BGRA format (as opposed to RGBA)
	// This is because most GPUs have a display controller which uses a little endian format
	// (It is also required on windows)
	// If we store the swapchain images as RGBA, a hidden conversion is required.
	// NOTE THAT we should NOT be rendering directly to the swapchain
	// We should be rendering to an FP16 (RBGA_F16) texture instead, then clamping to the framebuffer
	// (Gamma correction for example, should be a postprocess step and not done within the swapchain)
	vkb::Swapchain vkb_swapchain;

	size_t frame;
	std::vector<Frame> frames;

	uint32_t swapchain_image;
	std::vector<SwapchainImage> swapchain_images;

	bool init_glfw();
	void free_glfw();

	bool init_instance();
	bool init_surface();
	bool init_devices();
	bool init_queues();
	bool init_swapchain();
	bool init_swapchain_images();
	bool init_command_pool();
	bool init_frames();
	bool init_vulkan();

	void free_instance();
	void free_surface();
	void free_devices();
	void free_swapchain();
	void free_swapchain_images();
	void free_command_pool();
	void free_frames();
	void free_vulkan();

	bool draw_swapchain();
	bool should_close();
	void present_swapchain();
	void draw();

	Window();
	~Window();
};
