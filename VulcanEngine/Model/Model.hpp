#ifndef MODEL_HPP
#define MODEL_HPP

// dix
#include <Model/DixTexture/DixTexture.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/Buffer/DixBuffer.hpp>
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_SWIZZLE
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

                static std::vector <vk::VertexInputBindingDescription> getBindingDescriptions();
                static std::vector <vk::VertexInputAttributeDescription> getAttributeDescriptions();

                bool operator==(const Vertex& other) const {
                        return position == other.position &&
                                color == other.color &&
                                normal == other.normal &&
                                uv == other.uv;
                }
        };

        struct TextureInfo {
                vk::ImageView view { nullptr };
                vk::Sampler sampler { nullptr };
        };

        struct SubMesh {
                std::vector<Vertex> vertices;
                std::vector<uint32_t> indices;
                std::string texturePath;
                glm::vec3 baseColor{1.0f};
                uint32_t textureIndex{0};
                DixTexture texture;
        };

        struct Builder {
                std::vector <Vertex> vertices{};
                std::vector <uint32_t> indices{};
                std::vector<SubMesh> submeshes{};

                DixTexture texture;

                void loadModel(const std::string& filepath, EngineDevice& device);
        };

        Model(
                EngineDevice& dixDevice,
                const Model::Builder& builder,
                DixDescriptorPool& descriptorPool,
                DixDescriptorSetLayout& descriptorSetLayout
        );
        ~Model();

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        void bind(vk::CommandBuffer commandBuffer);
        void draw(vk::CommandBuffer commandBuffer);

        static std::unique_ptr <Model> createModelFromFile(
                EngineDevice& engineDevice,
                const std::string& filepath,
                DixDescriptorPool& descriptorPool,
                DixDescriptorSetLayout& descriptorSetLayout
        );

        vk::DescriptorSet getDescriptorSet() const { return m_descriptorSet; }

private:
        void createVertexBuffers(const std::vector <Vertex>& vertices);
        void createIndexBuffers(const std::vector <uint32_t>& indices);
        void createDescriptorSet(
                DixDescriptorPool& descriptorPool,
                DixDescriptorSetLayout& descriptorSetLayout
        );
private:
        EngineDevice& m_dixDevice;      // static?

        std::unique_ptr <DixBuffer> m_vertexBuffer;
        uint32_t vertexCount;

        bool m_hasIndexBuffer = false;
        std::unique_ptr <DixBuffer> m_indexBuffer;
        uint32_t indexCount;

        TextureInfo m_textureInfo{};
        DixTexture m_defaultTexture;
        vk::DescriptorSet m_descriptorSet{nullptr};
public:
        const TextureInfo& getTextureInfo() const { return m_textureInfo; }
};
}       // namespace dix
#endif // MODEL_HPP