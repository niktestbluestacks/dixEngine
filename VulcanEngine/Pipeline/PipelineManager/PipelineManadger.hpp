// libs
#include <vulkan/vulkan.h>

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>

#include <vector>

namespace dix {

// Forward declarations for Vulkan structures
struct PipelineConfigInfo;
class ShaderModule;

// Helper function to create a VkPipelineShaderStageCreateInfo from a shader module
VkPipelineShaderStageCreateInfo createShaderStage(VkShaderStageFlagBits stage, const ShaderModule& shaderModule);

// Helper function to create a VkPipelineVertexInputStateCreateInfo
VkPipelineVertexInputStateCreateInfo createVertexInputState(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);

// Helper function to create a VkPipelineInputAssemblyStateCreateInfo
VkPipelineInputAssemblyStateCreateInfo createInputAssemblyState(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

// Helper function to create a VkViewport and VkScissor for the viewport state
std::pair<VkViewport, VkRect2D> createViewportAndScissor(int width, int height);

// Helper function to create a VkPipelineRasterizationStateCreateInfo
VkPipelineRasterizationStateCreateInfo createRasterizationState(VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL, bool cullModeBack = true);

// Helper function to create a VkPipelineMultisampleStateCreateInfo
VkPipelineMultisampleStateCreateInfo createMultisampleState();

// Helper function to create a VkPipelineDepthStencilStateCreateInfo
VkPipelineDepthStencilStateCreateInfo createDepthStencilState(bool enableDepthTesting = true, bool enableDepthWriting = true, VkCompareOp compareOp = VK_COMPARE_OP_LESS);

// Helper function to create a VkPipelineColorBlendAttachmentState
VkPipelineColorBlendAttachmentState createColorBlendAttachmentState(bool blendEnable = true);

// Helper function to create a VkPipelineColorBlendStateCreateInfo
VkPipelineColorBlendStateCreateInfo createColorBlendState(const std::vector<VkPipelineColorBlendAttachmentState>& attachments);

class PipelineManager {
public:
    PipelineManager(EngineDevice& device, const PipelineConfigInfo& config);
    ~PipelineManager();

    // Not copyable or movable
    PipelineManager(const PipelineManager&) = delete;
    PipelineManager& operator=(const PipelineManager&) = delete;
    PipelineManager(PipelineManager&&) = delete;
    PipelineManager& operator=(PipelineManager&&) = delete;

    VkPipeline getPipeline() const { return pipeline; }

private:
    EngineDevice& device;
    VkPipeline pipeline;

    VkPipelineShaderStageCreateInfo vertShaderStageInfo;
    VkPipelineShaderStageCreateInfo fragShaderStageInfo;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineViewportStateCreateInfo viewportStateInfo;
    VkPipelineRasterizationStateCreateInfo rasterizerInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendingInfo;

    void createGraphicsPipeline(const PipelineConfigInfo& config);
};

struct PipelineConfigInfo {
    VkDevice device;
    VkRenderPass renderPass;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineViewportStateCreateInfo viewportStateInfo;
    VkPipelineRasterizationStateCreateInfo rasterizerInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendingInfo;
};

}  // namespace dix
