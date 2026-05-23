// dix
#include <Rendering/RenderSystem/SimpleRenderSystem/SimpleRenderSystem.hpp>

namespace dix {

SimpleRenderSystem::SimpleRenderSystem(
    EngineDevice& engineDevice,
    VkRenderPass renderPass,
    VkDescriptorSetLayout globalSetLayout,
    VkDescriptorSetLayout modelSetLayout)
    : DixRenderSystem(
        engineDevice,
        renderPass,
        globalSetLayout,
        modelSetLayout,
        DixRenderSystemConfig{
            .vertShaderPath = "SimpleShader/simple_shader.vert.spv",
            .fragShaderPath = "SimpleShader/simple_shader.frag.spv",
            // Default topology (TRIANGLE_LIST) — no override needed.
            // Default vertex input — Model::bind() provides buffers at draw time.
            .transformGameObject = [](void* push, GameObject& obj) {
                auto* p = static_cast<SimplePushConstantData*>(push);
                p->modelMatrix = obj.transform.mat4();
                p->normalMatrix = obj.transform.normalMatrix();
            },
            .pushConstantSize = sizeof(SimplePushConstantData),
        })
{}

}   // namespace dix
