// #include <Rendering/RenderSystem/ShadowMappingRenderSystem/ShadowMappingRenderSystem.hpp>

// namespace dix {

// ShadowMappingRenderSystem::ShadowMappingRenderSystem(
//     EngineDevice& device,
//     VkRenderPass renderPass,
//     VkDescriptorSetLayout globalSetLayout,
//     VkDescriptorSetLayout modelSetLayout
// ): DixRenderSystem(
//     device, 
//     renderPass, 
//     globalSetLayout, 
//     modelSetLayout,
//     DixRenderSystemConfig {
//         .vertShaderPath = "ShadowMapping/shadow_mapping.vert.spv",
//         .fragShaderPath = "ShadowMapping/shadow_mapping.frag.spv",
//         .transformGameObject = [](void* push, GameObject& obj, FrameInfo& frameInfo) {
//             auto* p = static_cast <ShadowMappingPushConstantData*>(push);
//             p->modelMatrix = obj.transform.mat4();
//         },
//         .pushConstantSize = sizeof(ShadowMappingPushConstantData),
//         .pipelineConfigInfo = std::move([]()->PipelineConfigInfo{
//             PipelineConfigInfo conf{};
//             conf.colorBlendAttachment.colorWriteMask = 0;
//             conf.depthStencilInfo.depthTestEnable = VK_TRUE;
//             conf.depthStencilInfo.depthWriteEnable = VK_TRUE;
//             conf.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
//             conf.rasterizetionInfo.depthBiasEnable = VK_TRUE;
//             conf.rasterizetionInfo.depthBiasConstantFactor = 1.19f;
//             conf.rasterizetionInfo.depthBiasSlopeFactor = 1.75f;
//             return conf;
//         }())
//     }
// ) {}

// }   // namespace dix