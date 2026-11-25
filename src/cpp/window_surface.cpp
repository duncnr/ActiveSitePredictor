#include <stdio.h>
#include <vulkan/vulkan_core.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
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
		VkImageView attachments[] = {swapchain_image_data.image_view, depth_image.image_view};
		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = this->vk_graphics_render_pass;
		framebuffer_info.attachmentCount = 2;
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
	free_depth_buffer();
	if (!init_swapchain()) return false;
	if (!init_swapchain_images()) return false;
	if (!init_depth_buffer()) return false;

	printf("Recreated Swapchain\n");
	return true;
}

bool Window::init_depth_buffer() {
	VkExtent3D extent_3d = {vkb_swapchain.extent.width, vkb_swapchain.extent.height, 1};

	// 1. Configure VkImageCreateInfo
	VkImageCreateInfo dimg_info = {};
	dimg_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	dimg_info.imageType = VK_IMAGE_TYPE_2D;
	dimg_info.format = VK_FORMAT_D32_SFLOAT; // TODO: this is hardcoded
	dimg_info.extent = extent_3d;
	dimg_info.mipLevels = 1;
	dimg_info.arrayLayers = 1;
	dimg_info.samples = VK_SAMPLE_COUNT_1_BIT;
	dimg_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	dimg_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vmaCreateImage(vma_allocator, &dimg_info, &dimg_allocinfo, &depth_image.image, &depth_image.allocation, nullptr) != VK_SUCCESS)
		return false;

	VkImageViewCreateInfo dview_info = {};
	dview_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	dview_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	dview_info.image = depth_image.image;
	dview_info.format = VK_FORMAT_D32_SFLOAT;
	dview_info.subresourceRange.baseMipLevel = 0;
	dview_info.subresourceRange.levelCount = 1;
	dview_info.subresourceRange.baseArrayLayer = 0;
	dview_info.subresourceRange.layerCount = 1;
	dview_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	if (vkCreateImageView(vkb_device.device, &dview_info, nullptr, &depth_image.image_view) != VK_SUCCESS)
		return false;

	return true;
}

void Window::free_depth_buffer() {
	this->vkb_device_dispatch.destroyImageView(depth_image.image_view, nullptr);
	vmaDestroyImage(vma_allocator, depth_image.image, depth_image.allocation);
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

	return true;
}

void Window::free_frames() {
	for (auto& frame_data : this->frames) {
        this->vkb_device_dispatch.destroySemaphore(frame_data.semaphore_available, nullptr);
        this->vkb_device_dispatch.destroyFence(frame_data.fence_inflight, nullptr);
    }
}

// Presentation

bool Window::draw_swapchain(const std::vector<glm::vec4>& points) {
	auto& frame_data = this->frames[this->frame]; // current frame data

	VkCommandBufferBeginInfo command_buffer_begin_info = {};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkCommandBuffer command_buffer = frame_data.command_buffer;
	if (this->vkb_device_dispatch.beginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS) {
		return false;
	}

	VkClearValue clear_color{{{0.0f, 0.0f, 1.0f, 1.0f}}};
	VkClearValue clear_depth = {}; clear_depth.depthStencil.depth = 1.0f;
	VkClearValue clear_values[] = {clear_color, clear_depth};

	VkRenderPassBeginInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = this->vk_graphics_render_pass;
	render_pass_info.framebuffer = this->swapchain_images[this->swapchain_image].framebuffer;
	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = this->vkb_swapchain.extent;
	render_pass_info.clearValueCount = 2;
	render_pass_info.pClearValues = &clear_values[0];

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

	VkDeviceSize offset = 0;
	this->vkb_device_dispatch.cmdBindVertexBuffers(frame_data.command_buffer, 0, 1, &mesh.vertex_buffer.buffer, &offset);
	auto view = glm::lookAt(glm::vec3{cos(curtime * 0.0001) * 2, sin(curtime * 0.0001) * 2, 0}, glm::vec3{}, glm::vec3{0, 0, 1});
	auto proj = glm::perspective(glm::radians(90.0f), static_cast<float>(vkb_swapchain.extent.width) / static_cast<float>(vkb_swapchain.extent.height), 0.1f, 100.0f);
	MeshPushConstants constants;
	constants.inv_view_proj = proj * view;
	for (glm::vec4 pos : points) {
		constants.pos = pos;
		this->vkb_device_dispatch.cmdPushConstants(frame_data.command_buffer, vk_graphics_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
		this->vkb_device_dispatch.cmdDraw(frame_data.command_buffer, mesh.vertices.size(), 1, 0, 0);
	}

	this->vkb_device_dispatch.cmdEndRenderPass(frame_data.command_buffer);

	if (this->vkb_device_dispatch.endCommandBuffer(frame_data.command_buffer) != VK_SUCCESS) {
		fprintf(stderr, "Failed to end command buffer\n");
		return false;
	}

	curtime += 1;

	return true;
}

void Window::present_swapchain(const std::vector<glm::vec4>& points) {
	auto& current_frame = this->frames[this->frame];

	this->vkb_device_dispatch.waitForFences(1, &current_frame.fence_inflight, VK_TRUE, UINT64_MAX);
	this->vkb_device_dispatch.resetFences(1, &current_frame.fence_inflight);

	VkResult result = this->vkb_device_dispatch.acquireNextImageKHR(this->vkb_swapchain, UINT64_MAX, current_frame.semaphore_available, VK_NULL_HANDLE, &this->swapchain_image);
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreate_swapchain();
		}
		else {
			fprintf(stderr, "Failed to aquire swapchain image\n");
		}
		return;
	}

	if (!draw_swapchain(points)) return;

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
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			recreate_swapchain();
		}
		else {
			fprintf(stderr, "Failed to present\n");
		}

		return;
	}
}

void Window::draw(const std::vector<glm::vec4>& points) {
	glfwPollEvents();
	present_swapchain(points);
	this->frame = (this->frame + 1) % FRAMES_IN_FLIGHT;
}
