#include "mesh.h"
#include "window.h"
#include <cmath>

bool Window::init_allocator() {
	VmaAllocatorCreateInfo allocator_create_info = {};
	allocator_create_info.physicalDevice = this->vkb_device.physical_device;
	allocator_create_info.device = this->vkb_device.device;
	allocator_create_info.instance = this->vkb_instance.instance;
	if (vmaCreateAllocator(&allocator_create_info, &(this->vma_allocator)) != VK_SUCCESS) {
		return false;
	}

	return true;
}

void Window::free_allocator() {
	vmaDestroyAllocator(this->vma_allocator);
}

// https://danielsieger.com/blog/2021/03/27/generating-spheres.html
std::vector<Vertex> uv_sphere(int n_slices, int n_stacks) {
	constexpr float PI = 3.14159265359f;
	std::vector<Vertex> mesh;
	std::vector<Vertex> vertices;

	auto v0 = Vertex{{0, 0, 1}};
	vertices.push_back(v0);

	for (int i = 0; i < n_stacks - 1; i++) {
		auto phi = PI * static_cast<double>(i + 1) / double(n_stacks);
		for (int j = 0; j < n_slices; j++) {
			auto theta = 2.0 * PI * static_cast<double>(j) / static_cast<double>(n_slices);
			auto x = std::sin(phi) * std::cos(theta);
			auto y = std::sin(phi) * std::sin(theta);
			auto z = std::cos(phi);
			vertices.push_back(Vertex{{x, y, z}});
		}
	}

	auto v1 = Vertex{{0, 0, -1}};
	vertices.push_back(v1);

	for (int i = 0; i < n_slices; ++i) {
		auto i0 = i + 1;
		auto i1 = (i + 1) % n_slices + 1;
		mesh.push_back(v0);
		mesh.push_back(vertices[i0]);
		mesh.push_back(vertices[i1]);
		i0 = i + n_slices * (n_stacks - 2) + 1;
		i1 = (i + 1) % n_slices + n_slices * (n_stacks - 2) + 1;
		mesh.push_back(v1);
		mesh.push_back(vertices[i1]);
		mesh.push_back(vertices[i0]);
	}

	for (int j = 0; j < n_stacks - 2; j++) {
		auto j0 = j * n_slices + 1;
		auto j1 = (j + 1) * n_slices + 1;
		for (int i = 0; i < n_slices; i++) {
			auto i0 = j0 + i;
			auto i1 = j0 + (i + 1) % n_slices;
			auto i2 = j1 + (i + 1) % n_slices;
			auto i3 = j1 + i;
			mesh.push_back(vertices[i0]);
			mesh.push_back(vertices[i2]);
			mesh.push_back(vertices[i1]);
			mesh.push_back(vertices[i0]);
			mesh.push_back(vertices[i3]);
			mesh.push_back(vertices[i2]);
		}
	}

	for (auto& v : mesh) {
		v.normal = glm::normalize(v.position);
	}

	return mesh;
}

bool Window::init_meshes() {
	this->mesh.vertices = uv_sphere(6, 6);
	if (!upload_mesh()) return false;

	return true;
}

bool Window::upload_mesh() {
	VkBufferCreateInfo buffer_create_info = {};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size = mesh.vertices.size() * sizeof(Vertex);
	buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
	VmaAllocationCreateInfo allocation_create_info = {};
	allocation_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	//allocate the buffer
	if (vmaCreateBuffer(vma_allocator, &buffer_create_info, &allocation_create_info,
		&mesh.vertex_buffer.buffer,
		&mesh.vertex_buffer.allocation,
		nullptr) != VK_SUCCESS)
		return false;

	void* data;
	vmaMapMemory(vma_allocator, mesh.vertex_buffer.allocation, &data);
	memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
	vmaUnmapMemory(vma_allocator, mesh.vertex_buffer.allocation);

	return true;
}

void Window::free_meshes() {
	// destroy mesh
	vmaDestroyBuffer(this->vma_allocator, mesh.vertex_buffer.buffer, mesh.vertex_buffer.allocation);
}
