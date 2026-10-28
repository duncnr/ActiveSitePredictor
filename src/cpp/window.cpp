#include <stdio.h>
#include "window.h"

// TODO: move vulkan info structs to a header file

bool Window::init_glfw() {
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	this->window = glfwCreateWindow(800, 600, "ASP Visual", nullptr, nullptr);
	if (!this->window) {
		glfwTerminate();
		fprintf(stderr, "GLFW Window Failed to Initialize\n");
		return false;
	}

	glfwMakeContextCurrent(this->window);
	return true;
}

void Window::free_glfw() {
	glfwDestroyWindow(this->window);
}

bool Window::init_instance() {
	vkb::InstanceBuilder builder;
	auto inst_ret = builder.set_app_name("ASP").request_validation_layers().use_default_debug_messenger().build();
	if (!inst_ret) {
		fprintf(stderr, "VkBootstrap InstanceBuilder Error\n");
		return false;
	}

	this->vkb_instance = inst_ret.value();
	return true;
}

void Window::free_instance() {
	vkb::destroy_instance(this->vkb_instance);
}

bool Window::init_devices() {
	vkb::PhysicalDeviceSelector selector{this->vkb_instance};
	auto phys_ret = selector.set_surface(this->vk_surface).set_minimum_version(1, 3).select();
	if (!phys_ret) {
		fprintf(stderr, "VkBootstrap PhysicalDevice Error: %s\n", phys_ret.error().message().c_str());
		const auto& detailed_reasons = phys_ret.detailed_failure_reasons();
		for (const std::string& reason : detailed_reasons) {
			fprintf(stderr, "\t%s\n", reason.c_str());
		}
		return false;
	}

	vkb::DeviceBuilder device_builder{phys_ret.value()};
	auto dev_ret = device_builder.build();
	if (!dev_ret) {
		fprintf(stderr, "VkBootstrap Device Error\n");
		return false;
	}

	this->vkb_device = dev_ret.value();
	this->vkb_device_dispatch = this->vkb_device.make_table();
	return true;
}

void Window::free_devices() {
	vkb::destroy_device(this->vkb_device);
}

bool Window::init_queues() {
	auto graphics_queue_ret = this->vkb_device.get_queue(vkb::QueueType::graphics);
	if (!graphics_queue_ret) {
		fprintf(stderr, "VkBootstrap VkQueue Error\n");
		return false;
	}
	this->vk_graphics_queue = graphics_queue_ret.value();

	auto present_queue_ret = this->vkb_device.get_queue(vkb::QueueType::present);
    if (!present_queue_ret.has_value()) {
    	fprintf(stderr, "VkBootstrap VkQueue Error\n");
        return false;
    }
    this->vk_present_queue = present_queue_ret.value();

	return true;
}

bool Window::init_command_pool() {
	VkCommandPoolCreateInfo pool_create_info = {};
	pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_create_info.queueFamilyIndex = this->vkb_device.get_queue_index(vkb::QueueType::graphics).value();

	if (this->vkb_device_dispatch.createCommandPool(&pool_create_info, nullptr, &this->vk_command_pool) != VK_SUCCESS) {
		fprintf(stderr, "VkBootstrap VkCommandPool Error\n");
		return false;
	}

	return true;
}

void Window::free_command_pool() {
	this->vkb_device_dispatch.destroyCommandPool(this->vk_command_pool, nullptr);
}

bool Window::init_vulkan() {
	if (!init_instance()) return false;
	printf("Vulkan Initiated Instance\n");
	if (!init_surface()) return false;
	printf("Vulkan Initiated Surface\n");
	if (!init_devices()) return false;
	printf("Vulkan Initiated Devices\n");
	if (!init_queues()) return false;
	printf("Vulkan Initiated Queues\n");
	if (!init_swapchain()) return false;
	printf("Vulkan Initiated Swapchain\n");
	if (!init_pipeline()) return false;
	printf("Vulkan Initiated Pipeline\n");
	if (!init_swapchain_images()) return false;
	printf("Vulkan Initiated Swapchain Images\n");
	if (!init_command_pool()) return false;
	printf("Vulkan Initiated Command Pool\n");
	if (!init_frames()) return false;
	printf("Vulkan Initiated Frames\n");

	printf("Vulkan Initialized\n");
	return true;
}

void Window::free_vulkan() {
	this->vkb_device_dispatch.deviceWaitIdle();
	free_frames();
	printf("Vulkan Freed Frames\n");
	free_command_pool();
	printf("Vulkan Freed Command Pool\n");
	free_swapchain_images();
	printf("Vulkan Freed Swapchain Images\n");
	free_pipeline();
	printf("Vulkan Freed Pipeline\n");
	free_swapchain();
	printf("Vulkan Freed Swapchain\n");
	free_devices();
	printf("Vulkan Freed Devices\n");
	free_surface();
	printf("Vulkan Freed Surface\n");
	free_instance();
	printf("Vulkan Freed Instance\n");
	printf("Vulkan Freed\n");
}

bool Window::should_close() {
	return glfwWindowShouldClose(this->window);
}

Window::Window() {
	init_glfw();
	init_vulkan();
}

Window::~Window() {
	free_vulkan();
	free_glfw();
}
