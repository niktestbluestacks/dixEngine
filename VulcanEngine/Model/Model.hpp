#ifndef MODEL_HPP
#define MODEL_HPP

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
// vector included already

namespace dix {
class Model {
public:
	struct Vertex {
		glm::vec2 position;
		glm::vec3 color;

		static std::vector <VkVertexInputBindingDescription> getBindingDescriptions();
		static std::vector <VkVertexInputAttributeDescription> getAttributeDescriptions();
	};

	Model(EngineDevice& dixDevice, const std::vector <Vertex>& verticies);
	~Model();

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	void bind(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);

private:
	void createVertexBuffers(const std::vector <Vertex>& verticies);
private:
	EngineDevice& m_dixDevice;
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	uint32_t vertexCount;
};
}	// namespace dix
#endif // MODEL_HPP