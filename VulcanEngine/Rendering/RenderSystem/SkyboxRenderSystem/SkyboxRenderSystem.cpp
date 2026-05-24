#include <Rendering/RenderSystem/SkyboxRenderSystem/SkyboxRenderSystem.hpp>

namespace dix {

SkyboxRenderSystem::SkyboxRenderSystem(
        EngineDevice& device,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout,
        VkDescriptorSetLayout modelSetLayout
    ):
    DixRenderSystem(
        device,
        renderPass,
        globalSetLayout,
        modelSetLayout,
        DixRenderSystemConfig{
            .vertShaderPath = "SkyboxShader/skybox_shader.vert.spv",
            .fragShaderPath = "SkyboxShader/skybox_shader.frag.spv",
            // Default topology (TRIANGLE_LIST) — no override needed.
            // Default vertex input — Model::bind() provides buffers at draw time.
            .transformGameObject = [](void* push, GameObject& obj, FrameInfo& frameInfo) {
                auto* p = static_cast <SkyboxPushContstantData*>(push);
                p->modelMatrix = glm::mat4{1.f};
                p->normalMatrix = glm::mat4{1.f};
            },
            .pushConstantSize = sizeof(SkyboxPushContstantData),
    }) {
    
}
}   // namespace dix