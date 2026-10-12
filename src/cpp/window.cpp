#include <stdio.h>
#include <vulkan/vulkan_core.h>
#include "window.h"

constexpr int FRAMES_IN_FLIGHT = 2;

bool Window::init_vulkan() {
	vkb::InstanceBuilder builder;
	auto inst_ret = builder.set_app_name("Example Vulkan Application").request_validation_layers().use_default_debug_messenger().build();
	if (!inst_ret) {
		fprintf(stderr, "VkBootstrap InstanceBuilder Error\n");
		return false;
	}

	vkb::Instance vkb_inst = inst_ret.value();
	vkb::PhysicalDeviceSelector selector{vkb_inst};
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkResult glfw_result = glfwCreateWindowSurface(vkb_inst, this->window, nullptr, &surface);
	if (glfw_result != VK_SUCCESS) {
		fprintf(stderr, "glfw Surface Error: %d\n", glfw_result);
		return false;
	}

	auto phys_ret = selector.set_surface(surface).set_minimum_version(1, 1).require_dedicated_transfer_queue().select();
	if (!phys_ret) {
		fprintf(stderr, "VkBootstrap PhysicalDevice Error: %s\n", phys_ret.error().message().c_str());
		return false;
	}

	vkb::DeviceBuilder device_builder{phys_ret.value()};
	auto dev_ret = device_builder.build();
	if (!dev_ret) {
		fprintf(stderr, "VkBootstrap Device Error\n");
		return false;
	}

	vkb::Device vkb_device = dev_ret.value();
	auto graphics_queue_ret = vkb_device.get_queue(vkb::QueueType::graphics);
	if (!graphics_queue_ret) {
		fprintf(stderr, "VkBootstrap VkQueue Error\n");
		return false;
	}

	VkQueue graphics_queue = graphics_queue_ret.value();
	vkb::SwapchainBuilder swapchain_builder{vkb_device};
    auto swap_ret = swapchain_builder.build();
    if (!swap_ret) {
        fprintf(stderr, "VkBootstrap Swapchain Error\n");
        return false;
    }

    // TODO: split these definitions
	this->vkb_instance = vkb_inst;
	this->vkb_device = vkb_device;
	this->vk_surface = surface;
	this->vk_graphics_queue = graphics_queue;
	this->vkb_swapchain = swap_ret.value();

	printf("Vulkan Initialized\n");
	return true;
}

void Window::update() {
	glfwPollEvents();
}

bool Window::should_close() {
	return glfwWindowShouldClose(this->window);
}

Window::Window() {
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	this->window = glfwCreateWindow(800, 600, "ASP Visual", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		fprintf(stderr, "GLFW Window Failed to Initialize\n");
		return;
	}

	glfwMakeContextCurrent(window);

	init_vulkan();
}

Window::~Window() {
	vkb::destroy_swapchain(this->vkb_swapchain);
	vkb::destroy_device(this->vkb_device);
	vkb::destroy_surface(this->vkb_instance, this->vk_surface);
	vkb::destroy_instance(this->vkb_instance);
	glfwDestroyWindow(this->window);
}
