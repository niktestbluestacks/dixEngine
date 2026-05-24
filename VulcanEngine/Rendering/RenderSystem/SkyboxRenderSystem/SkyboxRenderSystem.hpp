#ifndef SKYBOX_RENDER_SYSTEM_HPP
#define SKYBOX_RENDER_SYSTEM_HPP

#include <Rendering/RenderSystem/DixRenderSystem.hpp>

namespace dix {

struct SkyboxUbo {
    glm::mat4 projection;
    glm::mat4 view;
};

struct SkyboxPushContstantData {
    // Empty - skybox doesn't need push constants anymore
};

class SkyboxRenderSystem : public DixRenderSystem {
public:
    using DixRenderSystem::DixRenderSystem;
    SkyboxRenderSystem(
        EngineDevice& device,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout,
        VkDescriptorSetLayout modelSetLayout
    );
    DIX_DISABLE_COPY(SkyboxRenderSystem)
    ~SkyboxRenderSystem() = default;

    using Ubos = std::tuple<SkyboxUbo>;    
    using PushConstantData = SkyboxPushContstantData;

    static constexpr const char* Name() {
        return "SkyboxRenderSystem";
    }

    static constexpr std::tuple<VulkanRenderSystemFlagType, VulkanRenderSystemFlagType> 
    getVulkanFlags() {
        return std::make_tuple(
            VulkanRenderSystemFlagType{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT },
            VulkanRenderSystemFlagType{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT }
        );
    }

};
}   // namespace dix

#endif // SKYBOX_RENDER_SYSTEM_HPP