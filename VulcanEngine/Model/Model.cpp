#include "Model.hpp"

// std
#include <cassert>
#include <cstring>

namespace dix {
Model::Model(EngineDevice& dixDevice, const Model::Builder& builder) :
		m_dixDevice{ dixDevice } {
	createVertexBuffers(builder.vertices);
	createIndexBuffers(builder.indices);
}

Model::~Model() {
	vkDestroyBuffer(m_dixDevice.device(), m_vertexBuffer, nullptr);
	vkFreeMemory(m_dixDevice.device(), m_vertexBufferMemory, nullptr);

	if (m_hasIndexBuffer) {
		vkDestroyBuffer(m_dixDevice.device(), m_indexBuffer, nullptr);
		vkFreeMemory(m_dixDevice.device(), m_indexBufferMemory, nullptr);
	}
}

void Model::createVertexBuffers(const std::vector<Vertex>& vertices) {
	vertexCount = static_cast <uint32_t> (vertices.size());
	assert(vertexCount >= 3 && "Vertex count must be at least 3");
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	m_dixDevice.createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	void* data;
	vkMapMemory(m_dixDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), static_cast <size_t> (bufferSize));
	vkUnmapMemory(m_dixDevice.device(), stagingBufferMemory);

	m_dixDevice.createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_vertexBuffer,
		m_vertexBufferMemory
	);

	m_dixDevice.copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

	vkDestroyBuffer(m_dixDevice.device(), stagingBuffer, nullptr);
	vkFreeMemory(m_dixDevice.device(), stagingBufferMemory, nullptr);
}

void Model::createIndexBuffers(const std::vector<uint32_t>& indices) {
	indexCount = static_cast <uint32_t> (indices.size());
	m_hasIndexBuffer = indexCount > 0;
	if (!m_hasIndexBuffer) {
		return;
	}
	VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	m_dixDevice.createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	void* data;
	vkMapMemory(m_dixDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), static_cast <size_t> (bufferSize));
	vkUnmapMemory(m_dixDevice.device(), stagingBufferMemory);

	m_dixDevice.createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_indexBuffer,
		m_indexBufferMemory
	);

	m_dixDevice.copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

	vkDestroyBuffer(m_dixDevice.device(), stagingBuffer, nullptr);
	vkFreeMemory(m_dixDevice.device(), stagingBufferMemory, nullptr);
}

void Model::draw(VkCommandBuffer commandBuffer) {
	if (m_hasIndexBuffer) {
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
	}
	else {
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}
}

void Model::bind(VkCommandBuffer commandBuffer) {
	VkBuffer buffers[] = { m_vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

	if (m_hasIndexBuffer) {
		vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	}
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescriptions;
}
std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
	
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location= 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	return attributeDescriptions;
}
}	// namespace dix