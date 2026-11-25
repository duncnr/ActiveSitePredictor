#pragma once

#include <vector>
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>
#include <VkBootstrapDispatch.h>
#include "vk_mem_alloc.h"
#include "mesh.h"
#include <glm/mat4x4.hpp>

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

struct DepthImage {
	VkImage image;
	VmaAllocation allocation;
	VkImageView image_view;
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
	// For simplicity however, we're just going to render directly
	vkb::Swapchain vkb_swapchain;

	// unless we start doing triple buffering, we only need one of these,
	// becasue the depth buffer isn't being presented to the screen
	DepthImage depth_image;

	VkRenderPass vk_graphics_render_pass;
	VkPipelineLayout vk_graphics_pipeline_layout;
	VkPipeline vk_graphics_pipeline;

	size_t frame;
	std::vector<Frame> frames;
	size_t curtime;

	uint32_t swapchain_image;
	std::vector<SwapchainImage> swapchain_images;

	VmaAllocator vma_allocator;
	Mesh mesh;

	glm::mat4x4 camera_transform = {};

	bool init_glfw();
	void free_glfw();

	VkShaderModule init_shader_module(const std::string& path);
	void free_shader_module(VkShaderModule module);

	bool init_instance();
	bool init_surface();
	bool init_devices();
	bool init_queues();
	bool init_swapchain();
	bool init_swapchain_images();
	bool init_depth_buffer();
	bool init_command_pool();
	bool init_frames();
	bool init_pipeline();
	bool init_allocator();
	bool init_meshes();

	bool upload_mesh();
	bool recreate_swapchain();

	void free_instance();
	void free_surface();
	void free_devices();
	void free_swapchain();
	void free_swapchain_images();
	void free_depth_buffer();
	void free_command_pool();
	void free_frames();
	void free_pipeline();
	void free_allocator();
	void free_meshes();

	bool init_vulkan();
	void free_vulkan();

	bool should_close();
	bool draw_swapchain(const std::vector<glm::vec4>& points);
	void present_swapchain(const std::vector<glm::vec4>& points);
	void draw(const std::vector<glm::vec4>& points);

	Window();
	~Window();
};
