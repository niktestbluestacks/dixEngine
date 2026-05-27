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

using SkyboxRenderSystemBindings =
    std::tuple<UniformBinding<SkyboxUbo, 0, vk::ShaderStageFlagBits::eVertex>,
               SamplerBinding<1, vk::ShaderStageFlagBits::eFragment> >;

class SkyboxRenderSystem
    : public DixRenderSystem,
      public RenderSystemTraits<SkyboxRenderSystemBindings> {
   public:
    using PushConstantData = SkyboxPushContstantData;
    using DixRenderSystem::DixRenderSystem;
    SkyboxRenderSystem(EngineDevice& device, vk::RenderPass renderPass,
                       vk::DescriptorSetLayout globalSetLayout,
                       vk::DescriptorSetLayout modelSetLayout);
    DIX_DISABLE_COPY(SkyboxRenderSystem)
    ~SkyboxRenderSystem() = default;

    static constexpr const char* Name() { return "SkyboxRenderSystem"; }
};
}  // namespace dix

#endif  // SKYBOX_RENDER_SYSTEM_HPP