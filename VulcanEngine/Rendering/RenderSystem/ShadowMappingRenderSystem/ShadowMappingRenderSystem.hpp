// #ifndef SHADOW_MAPPING_RENDER_SYSTEM
// #define SHADOW_MAPPING_RENDER_SYSTEM

// // dix
// #include <Rendering/RenderSystem/DixRenderSystem.hpp>

// // libs
// #include <vulkan/vulkan.h>
// #include <vk_mem_alloc.h>

// namespace dix {

// struct ShadowMappingPushConstantData {
//     alignas(16) glm::mat4 modelMatrix { 1.f };
// };

// struct ShadowUbo {
//     alignas(16) glm::mat4 lightSpaceMatrix;
// };

// class ShadowMappingRenderSystem : public DixRenderSystem {
// public:
//     using DixRenderSystem::DixRenderSystem;
//     using Ubos = std::tuple<ShadowUbo>;
//     ShadowMappingRenderSystem(
//         EngineDevice& device,
//         VkRenderPass renderPass,
//         VkDescriptorSetLayout globalSetLayout,
//         VkDescriptorSetLayout modelSetLayout
//     );

//     ~ShadowMappingRenderSystem() = default;

//     static constexpr const char* Name() {
//         return "ShadowMappingRenderSystem";
//     }

//     std::tuple <VulkanRenderSystemFlagType> getVulkanFlags() {
//         return std::make_tuple(
//             VulkanRenderSystemFlagType{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//             VK_SHADER_STAGE_VERTEX_BIT }
//         );
//     }

//     void renderGameObjects(FrameInfo& frameInfo, std::vector<GameObject>&
//     gameObjects) override; void setLightSpaceMatrix(const glm::mat4& matrix);

//     VkImageView getDepthImageView() const { return m_depthImageView; }

//     DIX_DISABLE_COPY(ShadowMappingRenderSystem)
// protected:
//     void createShadowResources(VkExtent2D swapChainExtent);
//     void cleanupShadowResources();
//     void createShadowPipeline();
//     void recordShadowPass(VkCommandBuffer commandBuffer, FrameInfo&
//     frameInfo);

//     VkExtent2D m_swapChainExtent;

//     // Internal resources specific to Shadow Mapping
//     VkRenderPass m_shadowRenderPass{ VK_NULL_HANDLE };
//     VkFramebuffer m_shadowFramebuffer{ VK_NULL_HANDLE };
//     VkImage m_depthImage{ VK_NULL_HANDLE };
//     VkImageView m_depthImageView{ VK_NULL_HANDLE };
//     VmaAllocation m_depthImageAllocation{ VK_NULL_HANDLE };

//     glm::mat4 m_lightSpaceMatrix{ 1.0f };

// };
// }   // namespace dix

// #endif // SHADOW_MAPPING_RENDER_SYSTEM