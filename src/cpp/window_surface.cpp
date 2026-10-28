#include <stdio.h>
#include <vulkan/vulkan_core.h>
#include "window.h"

#define FRAMES_IN_FLIGHT 2

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
		VkImageView attachments[] = { swapchain_image_data.image_view };
		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = this->vk_graphics_render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = this->vkb_swapchain.extent.width;
		framebuffer_info.height = this->vkb_swapchain.extent.height;
		framebuffer_info.layers = 1;
		if (this->vkb_device_dispatch.createFramebuffer(&framebuffer_info, nullptr, &swapchain_image_data.framebuffer) != VK_SUCCESS) {
			fprintf(stderr, "VkBootstrap VkFramebuffer Error\n");
			return false;
		}

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

bool Window::recreate_swapchain() {
	this->vkb_device_dispatch.deviceWaitIdle();

	free_swapchain_images();
	free_swapchain();
	if (!init_swapchain()) return false;
	if (!init_swapchain_images()) return false;

	return true;
}

bool Window::init_frames() {
	this->frames.resize(FRAMES_IN_FLIGHT);

	// Frame setup
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

	// Framebuffer setup
	this->swapchain_images.resize(this->vkb_swapchain.image_count);

	auto image_and_views = this->vkb_swapchain.get_images_and_image_views();

	for (uint32_t i = 0; i < this->vkb_swapchain.image_count; i++) {
		this->swapchain_images[i].image = image_and_views->first[i];
		this->swapchain_images[i].image_view = image_and_views->second[i];
	}

	for (auto& swapchain_image_data : this->swapchain_images) {
		VkImageView attachments[] = { swapchain_image_data.image_view };

		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = this->vk_graphics_render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = this->vkb_swapchain.extent.width;
		framebuffer_info.height = this->vkb_swapchain.extent.height;
		framebuffer_info.layers = 1;

		if (this->vkb_device_dispatch.createFramebuffer(&framebuffer_info, nullptr, &swapchain_image_data.framebuffer) != VK_SUCCESS) {
			fprintf(stderr, "VkBootstrap VkFramebuffer Error\n");
			return false;
		}

		VkSemaphoreCreateInfo semaphore_info = {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (this->vkb_device_dispatch.createSemaphore(&semaphore_info, nullptr, &swapchain_image_data.semaphore_finished) != VK_SUCCESS) {
			fprintf(stderr, "VkBootstrap VkFramebuffer VkSemaphore Error\n");
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

// Presentation

bool Window::draw_swapchain() {
	auto& frame_data = this->frames[this->frame]; // current frame data

	VkCommandBufferBeginInfo command_buffer_begin_info = {};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkCommandBuffer command_buffer = frame_data.command_buffer;
	if (this->vkb_device_dispatch.beginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS) {
		return false;
	}

	VkRenderPassBeginInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = this->vk_graphics_render_pass;
	render_pass_info.framebuffer = this->swapchain_images[this->swapchain_image].framebuffer;
	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = this->vkb_swapchain.extent;
	VkClearValue clearColor{ { { 0.0f, 0.0f, 1.0f, 1.0f } } };
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
	scissor.extent = this->vkb_swapchain.extent;

	this->vkb_device_dispatch.cmdSetViewport(frame_data.command_buffer, 0, 1, &viewport);
	this->vkb_device_dispatch.cmdSetScissor(frame_data.command_buffer, 0, 1, &scissor);
	this->vkb_device_dispatch.cmdBeginRenderPass(frame_data.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	this->vkb_device_dispatch.cmdBindPipeline(frame_data.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->vk_graphics_pipeline);
	this->vkb_device_dispatch.cmdDraw(frame_data.command_buffer, 3, 1, 0, 0);
	this->vkb_device_dispatch.cmdEndRenderPass(frame_data.command_buffer);

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
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreate_swapchain();
		}
		else {
			fprintf(stderr, "Failed to aquire swapchain image\n");
		}
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
