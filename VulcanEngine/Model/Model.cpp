#include "Model.hpp"

// std
#include <cassert>
#include <cstring>

namespace dix {
Model::Model(EngineDevice& dixDevice, const std::vector<Vertex>& verticies) :
		m_dixDevice{ dixDevice } {
	createVertexBuffers(verticies);
}

Model::~Model() {
	vkDestroyBuffer(m_dixDevice.device(), m_vertexBuffer, nullptr);
	vkFreeMemory(m_dixDevice.device(), m_vertexBufferMemory, nullptr);

}

void Model::createVertexBuffers(const std::vector<Vertex>& verticies) {
	vertexCount = static_cast <uint32_t> (verticies.size());
	assert(vertexCount >= 3 && "Vertex count must be at least 3");
	VkDeviceSize buffersize = sizeof(verticies[0]) * vertexCount;
	m_dixDevice.createBuffer(
		buffersize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_vertexBuffer,
		m_vertexBufferMemory);

	void* data;
	vkMapMemory(m_dixDevice.device(), m_vertexBufferMemory, 0, buffersize, 0, &data);
	memcpy(data, verticies.data(), static_cast <size_t> (buffersize));
	vkUnmapMemory(m_dixDevice.device(), m_vertexBufferMemory);
}

void Model::bind(VkCommandBuffer commandBuffer) {
	VkBuffer buffers[] = { m_vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}
void Model::draw(VkCommandBuffer commandBuffer) {
	vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
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
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	return attributeDescriptions;
}
}	// namespace dix