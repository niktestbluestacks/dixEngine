#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <Pipeline/EngineDevice/EngineDevice.hpp>

#include <string>
#include <vector>
#include <cstdint>
#include <vulkan/vulkan.h>

// optional custom vertex input descriptions
#include <vector>
#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>


namespace dix {

struct PipelineConfigInfo {
	PipelineConfigInfo() = default;
	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

	VkPipelineViewportStateCreateInfo viewportInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizetionInfo;
	VkPipelineMultisampleStateCreateInfo multisampleStateInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	std::vector <VkDynamicState> dynamicStateEnables;
	VkPipelineDynamicStateCreateInfo dynamicStateInfo;
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;

	// optional custom vertex input descriptions
	std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
};

class Pipeline {
public:
	Pipeline(
		EngineDevice& device, 
		const std::string& vertFilepath, 
		const std::string& fragFilepath, 
		const PipelineConfigInfo& configInfo);

	~Pipeline();

	Pipeline(const Pipeline&) = delete;
	Pipeline operator=(const Pipeline&) = delete;

	void bind(VkCommandBuffer commandBuffer);
	static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

private:
	static std::vector <char> readFile(const std::string& filepath);

	void createGraphicsPipeline(
		const std::string& vertShaderCode, 
		const std::string& fragShaderCode,
		const PipelineConfigInfo& pipelineInfo);

	void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

private:

	EngineDevice& dixdevice;
	VkPipeline graphicsPipeline;
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
}; // class Pipeline
} // namespace dix

#endif // PIPELINE_HPP