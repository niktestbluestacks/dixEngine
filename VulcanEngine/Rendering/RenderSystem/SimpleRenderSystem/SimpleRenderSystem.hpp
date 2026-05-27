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

using SimpleRenderSystemBindings = std::tuple<
    UniformBinding<SimpleUbo, 0, vk::ShaderStageFlagBits::eVertex>,
    SamplerBinding<1,            vk::ShaderStageFlagBits::eFragment>
>;

class SimpleRenderSystem:
    public DixRenderSystem,
    public RenderSystemTraits<SimpleRenderSystemBindings> {
public:
    using PushConstantData = SimplePushConstantData;

    SimpleRenderSystem(
        EngineDevice& engineDevice,
        vk::RenderPass renderPass,
        vk::DescriptorSetLayout globalSetLayout,
        vk::DescriptorSetLayout modelSetLayout
    );

    ~SimpleRenderSystem() = default;

    static constexpr const char* Name() { return "SimpleRenderSystem"; }

    DIX_DISABLE_COPY(SimpleRenderSystem)
};

}   // namespace dix
#endif // SIMPLE_RENDER_SYSTEM_HPP