#ifndef MODEL_HPP
#define MODEL_HPP

// dix
#include <Model/DixTexture/DixTexture.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/Buffer/DixBuffer.hpp>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
// vector included already

namespace dix {
class Model {
public:
	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
		glm::vec3 normal{};
		glm::vec2 uv{};

		static std::vector <VkVertexInputBindingDescription> getBindingDescriptions();
		static std::vector <VkVertexInputAttributeDescription> getAttributeDescriptions();

		bool operator==(const Vertex& other) const {
			return position == other.position &&
				color == other.color &&
				normal == other.normal &&
				uv == other.uv;
		}
	};

	struct TextureInfo {
		VkImageView view;
		VkSampler sampler;
	};

	struct Builder {
		std::vector <Vertex> vertices{};
		std::vector <uint32_t> indices{};

		void loadModel(const std::string& filepath, EngineDevice& device);

		DixTexture texture;
	};

	Model(EngineDevice& dixDevice, const Model::Builder& builder);
	~Model();

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	void bind(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);

	static std::unique_ptr <Model> createModelFromFile(
		EngineDevice& engineDevice, 
		const std::string& filepath);

private:
	void createVertexBuffers(const std::vector <Vertex>& vertices);
	void createIndexBuffers(const std::vector <uint32_t>& indices);
private:
	EngineDevice& m_dixDevice;	// static?

	std::unique_ptr <DixBuffer> m_vertexBuffer;
	uint32_t vertexCount;

	bool m_hasIndexBuffer = false;
	std::unique_ptr <DixBuffer> m_indexBuffer;
	uint32_t indexCount;

	TextureInfo m_textureInfo{};

public:
	const TextureInfo& getTextureInfo() const { return m_textureInfo; }
};
}	// namespace dix
#endif // MODEL_HPP