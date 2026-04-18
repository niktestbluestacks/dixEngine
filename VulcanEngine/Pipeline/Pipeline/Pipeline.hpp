#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <Pipeline/EngineDevice/EngineDevice.hpp>

#include <string>
#include <vector>
#include <cstdint>

namespace dix {

struct PipelineConfigInfo {
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizetionInfo;
	VkPipelineMultisampleStateCreateInfo multisampleStateInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
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
	void operator=(const Pipeline&) = delete;

	void bind(VkCommandBuffer commandBuffer);
	static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

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