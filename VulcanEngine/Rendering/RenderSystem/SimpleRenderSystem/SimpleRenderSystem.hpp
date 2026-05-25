#ifndef SIMPLE_RENDER_SYSTEM_HPP
#define SIMPLE_RENDER_SYSTEM_HPP

// dix
#include <Rendering/RenderSystem/DixRenderSystem.hpp>

namespace dix {

struct SimpleUbo {
    alignas(16) glm::mat4 projectionView{ 1.f };
    alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{ 100.f, 300.f, -100.f });
};

struct SimplePushConstantData {
    glm::mat4 modelMatrix{ 1.f };
    glm::mat4 normalMatrix{ 1.f };
};

class SimpleRenderSystem : public DixRenderSystem {
public:
    using Ubos = std::tuple<SimpleUbo>;
    using PushConstantData = SimplePushConstantData;

    SimpleRenderSystem(
        EngineDevice& engineDevice,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout,
        VkDescriptorSetLayout modelSetLayout
    );

    ~SimpleRenderSystem() = default;

    static constexpr const char* Name() { return "SimpleRenderSystem"; }

    static constexpr std::tuple<VulkanRenderSystemFlagType, VulkanRenderSystemFlagType>
    getVulkanFlags() {
        return std::make_tuple(
            VulkanRenderSystemFlagType{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT },
            VulkanRenderSystemFlagType{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT }
        );
    }

    DIX_DISABLE_COPY(SimpleRenderSystem)
};

}   // namespace dix
#endif // SIMPLE_RENDER_SYSTEM_HPP
