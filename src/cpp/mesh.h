#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include <glm/mat4x4.hpp>

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct VertexInputDescription {
	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;

    static VertexInputDescription get_vertex_description();
};

struct Mesh {
	std::vector<Vertex> vertices;
	AllocatedBuffer vertex_buffer;
};

struct MeshPushConstants {
	glm::mat4 inv_view_proj;
	glm::vec4 pos;
};
