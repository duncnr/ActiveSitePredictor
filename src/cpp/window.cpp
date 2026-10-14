#include <stdio.h>
#include <vulkan/vulkan_core.h>
#include "window.h"

#define FRAMES_IN_FLIGHT 2

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

bool Window::init_surface() {
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkResult glfw_result = glfwCreateWindowSurface(this->vkb_instance, this->window, nullptr, &surface);
	if (glfw_result != VK_SUCCESS) {
		fprintf(stderr, "glfw Surface Error: %d\n", glfw_result);
		return false;
	}

	this->vk_surface = surface;
	return true;
}

void Window::free_surface() {
	vkb::destroy_surface(this->vkb_instance, this->vk_surface);
}

bool Window::init_devices() {
	vkb::PhysicalDeviceSelector selector{this->vkb_instance};
	auto phys_ret = selector.set_surface(this->vk_surface).set_minimum_version(1, 3).require_dedicated_transfer_queue().select();
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

bool Window::init_swapchain() {
	vkb::SwapchainBuilder swapchain_builder{this->vkb_device};
	auto swap_ret = swapchain_builder.build();
	if (!swap_ret) {
		fprintf(stderr, "VkBootstrap Swapchain Error\n");
		return false;
	}

	this->vkb_swapchain = swap_ret.value();
	return true;
}

void Window::free_swapchain() {
	vkb::destroy_swapchain(this->vkb_swapchain);
}

bool Window::init_swapchain_images() {
	auto& swapchain_images = this->swapchain_images;
	swapchain_images.resize(this->vkb_swapchain.image_count);

	auto image_and_views = this->vkb_swapchain.get_images_and_image_views();
	for (uint32_t i = 0; i < this->vkb_swapchain.image_count; i++) {
		swapchain_images[i].image = image_and_views->first[i];
		swapchain_images[i].image_view = image_and_views->second[i];
	}

	for (auto& swapchain_image_data : swapchain_images) {
		/*
		VkImageView attachments[] = { swapchain_image_data.swapchain_image_view };
		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = data.render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = this->vkb_swapchain.extent.width;
		framebuffer_info.height = this->vkb_swapchain.extent.height;
		framebuffer_info.layers = 1;
		if (this->vkb_device_dispatch.createFramebuffer(&framebuffer_info, nullptr, &swapchain_image_data.framebuffer) != VK_SUCCESS) {
			return -1;
		}*/

		VkSemaphoreCreateInfo semaphore_info = {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		if (this->vkb_device_dispatch.createSemaphore(&semaphore_info, nullptr, &swapchain_image_data.semaphore_finished) != VK_SUCCESS) {
			fprintf(stderr, "VkBootstrap Swapchain VkSemaphore Error\n");
			return false;
		}
	}

	return true;
}

void Window::free_swapchain_images() {
	for (auto& image_data : this->swapchain_images) {
		this->vkb_device_dispatch.destroySemaphore(image_data.semaphore_finished, nullptr);
		this->vkb_device_dispatch.destroyImageView(image_data.image_view, nullptr);
		this->vkb_device_dispatch.destroyFramebuffer(image_data.framebuffer, nullptr);
	}
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

bool Window::init_frames() {
	this->frames.resize(FRAMES_IN_FLIGHT);

	for (auto& frame : this->frames) {
		VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
		command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.commandPool = this->vk_command_pool;
		command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandBufferCount = 1;
		if (this->vkb_device_dispatch.allocateCommandBuffers(&command_buffer_allocate_info, &frame.command_buffer) != VK_SUCCESS) {
			fprintf(stderr, "VkBootstrap VkCommandBuffer Error\n");
			return false;
		}

		VkSemaphoreCreateInfo semaphore_create_info = {};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		if (this->vkb_device_dispatch.createSemaphore(&semaphore_create_info, nullptr, &frame.semaphore_available) != VK_SUCCESS) {
			fprintf(stderr, "VkBootstrap VkSemaphore Error\n");
			return false;
		}

		VkFenceCreateInfo fence_create_info = {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		if (this->vkb_device_dispatch.createFence(&fence_create_info, nullptr, &frame.fence_inflight) != VK_SUCCESS) {
			fprintf(stderr, "VkBootstrap VkFence Error\n");
			return false;
		}
	}

	return true;
}

void Window::free_frames() {
	for (auto& frame_data : this->frames) {
        this->vkb_device_dispatch.destroySemaphore(frame_data.semaphore_available, nullptr);
        this->vkb_device_dispatch.destroyFence(frame_data.fence_inflight, nullptr);
    }
}

bool Window::init_vulkan() {
	if (!init_instance()) return false;
	if (!init_surface()) return false;
	if (!init_devices()) return false;
	if (!init_queues()) return false;
	if (!init_swapchain()) return false;
	if (!init_swapchain_images()) return false;
	if (!init_command_pool()) return false;
	if (!init_frames()) return false;

	printf("Vulkan Initialized\n");
	return true;
}

void Window::free_vulkan() {
	this->vkb_device_dispatch.deviceWaitIdle();
	free_frames();
	free_command_pool();
	free_swapchain_images();
	free_swapchain();
	free_devices();
	free_surface();
	free_instance();
	printf("Vulkan Freed\n");
}

bool Window::draw_swapchain() {
	auto& frame_data = this->frames[this->frame]; // current frame data

	VkCommandBufferBeginInfo command_buffer_begin_info = {};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkCommandBuffer command_buffer = frame_data.command_buffer;
	if (this->vkb_device_dispatch.beginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS) {
		return false;
	}
	/*
	VkRenderPassBeginInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = data.render_pass;
	render_pass_info.framebuffer = data.swapchain_image_data.at(data.current_swapchain_image).framebuffer;
	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = init.swapchain.extent;
	VkClearValue clearColor{ { { 0.0f, 0.0f, 0.0f, 1.0f } } };
	render_pass_info.clearValueCount = 1;
	render_pass_info.pClearValues = &clearColor;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(this->vkb_swapchain.extent.width);
	viewport.height = static_cast<float>(this->vkb_swapchain.extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = this->vkb_swapchain.swapchain.extent;

	init.disp.cmdSetViewport(current_frame_data.command_buffer, 0, 1, &viewport);
	init.disp.cmdSetScissor(current_frame_data.command_buffer, 0, 1, &scissor);

	init.disp.cmdBeginRenderPass(current_frame_data.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	init.disp.cmdBindPipeline(current_frame_data.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data.graphics_pipeline);

	init.disp.cmdDraw(current_frame_data.command_buffer, 3, 1, 0, 0);

	init.disp.cmdEndRenderPass(current_frame_data.command_buffer);*/

	VkClearColorValue clear_color_value = {0.0f, 0.0f, 1.0f, 1.0f};
	VkImageSubresourceRange image_subresource_range {};
	image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_subresource_range.baseMipLevel = 0;
	image_subresource_range.levelCount = VK_REMAINING_MIP_LEVELS;
	image_subresource_range.baseArrayLayer = 0;
	image_subresource_range.layerCount = VK_REMAINING_ARRAY_LAYERS;

	this->vkb_device_dispatch.cmdClearColorImage(
	   	command_buffer,
	   	this->swapchain_images[this->swapchain_image].image,
		VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR,
		&clear_color_value,
		1,
		&image_subresource_range
	);

	if (this->vkb_device_dispatch.endCommandBuffer(frame_data.command_buffer) != VK_SUCCESS) {
		fprintf(stderr, "Failed to end command buffer\n");
		return false;
	}

	return true;
}

void Window::present_swapchain() {
	auto& current_frame = this->frames[this->frame];

	this->vkb_device_dispatch.waitForFences(1, &current_frame.fence_inflight, VK_TRUE, UINT64_MAX);
	this->vkb_device_dispatch.resetFences(1, &current_frame.fence_inflight);

	VkResult result = this->vkb_device_dispatch.acquireNextImageKHR(this->vkb_swapchain, UINT64_MAX, current_frame.semaphore_available, VK_NULL_HANDLE, &this->swapchain_image);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Failed to aquire swapchain image\n");
		//free_swapchain_images();
		//free_swapchain();
		//init_swapchain();
		//init_swapchain_images();
		return;
	}

    if (!draw_swapchain()) return;

    auto& current_swapchain_image = this->swapchain_images[this->swapchain_image];
    VkSemaphore wait_semaphores[] = {current_frame.semaphore_available};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signal_semaphores[] = {current_swapchain_image.semaphore_finished};
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &current_frame.command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if (this->vkb_device_dispatch.queueSubmit(this->vk_graphics_queue, 1, &submit_info, current_frame.fence_inflight) != VK_SUCCESS) {
        fprintf(stderr, "Failed to submit queue");
        return;
    }

    VkSwapchainKHR swap_chains[] = {this->vkb_swapchain};
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &this->swapchain_image;

    result = this->vkb_device_dispatch.queuePresentKHR(this->vk_present_queue, &present_info);
    if (result != VK_SUCCESS) {
    	fprintf(stderr, "Failed to present\n");
        return;
    }
}

void Window::draw() {
	glfwPollEvents();
	present_swapchain();
	this->frame = (this->frame + 1) % FRAMES_IN_FLIGHT;
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
