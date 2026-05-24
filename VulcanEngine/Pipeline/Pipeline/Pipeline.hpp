#ifndef PIPELINE_HPP
#define PIPELINE_HPP

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>

// libs
#include <vulkan/vulkan.hpp>

// std
#include <string>
#include <vector>
#include <cstdint>


namespace dix {

struct PipelineConfigInfo {
	PipelineConfigInfo() {
		this->inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		this->inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		this->inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		this->viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		this->viewportInfo.viewportCount = 1;
		this->viewportInfo.pViewports = nullptr;
		this->viewportInfo.scissorCount = 1;
		this->viewportInfo.pScissors = nullptr;

		this->rasterizetionInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		this->rasterizetionInfo.depthClampEnable = VK_FALSE;
		this->rasterizetionInfo.rasterizerDiscardEnable = VK_FALSE;
		this->rasterizetionInfo.polygonMode = VK_POLYGON_MODE_FILL;
		this->rasterizetionInfo.lineWidth = 1.0f;
		this->rasterizetionInfo.cullMode = VK_CULL_MODE_NONE;
		this->rasterizetionInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		this->rasterizetionInfo.depthBiasEnable = VK_FALSE;
		this->rasterizetionInfo.depthBiasConstantFactor = 0.0f;		// optional
		this->rasterizetionInfo.depthBiasClamp = 0.0f;					// optional
		this->rasterizetionInfo.depthBiasSlopeFactor = 0.0f;			// optional

		this->multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		this->multisampleStateInfo.sampleShadingEnable = VK_FALSE;
		this->multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		this->multisampleStateInfo.minSampleShading = 1.0f;			// optional
		this->multisampleStateInfo.pSampleMask = nullptr;				// optional
		this->multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;	// optional
		this->multisampleStateInfo.alphaToOneEnable = VK_FALSE;		// optional

		this->colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		this->colorBlendAttachment.blendEnable = VK_FALSE;
		this->colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;	// optional
		this->colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;	// optional
		this->colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;				// optional
		this->colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;	// optional
		this->colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;	// optional
		this->colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;				// optional

		this->colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		this->colorBlendInfo.logicOpEnable = VK_FALSE;
		this->colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;	// optional
		this->colorBlendInfo.attachmentCount = 1;
		this->colorBlendInfo.pAttachments = &this->colorBlendAttachment;
		this->colorBlendInfo.blendConstants[0] = 0.0f;		// optional
		this->colorBlendInfo.blendConstants[1] = 0.0f;		// optional
		this->colorBlendInfo.blendConstants[2] = 0.0f;		// optional
		this->colorBlendInfo.blendConstants[3] = 0.0f;		// optional

		this->depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		this->depthStencilInfo.depthTestEnable = VK_TRUE;
		this->depthStencilInfo.depthWriteEnable = VK_TRUE;
		this->depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		this->depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		this->depthStencilInfo.minDepthBounds = 0.0f;	// optional
		this->depthStencilInfo.maxDepthBounds = 1.0f;	// optional
		this->depthStencilInfo.stencilTestEnable = VK_FALSE;
		this->depthStencilInfo.front = {};				// optional
		this->depthStencilInfo.back = {};				// optional

		this->dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		this->dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		this->dynamicStateInfo.pDynamicStates = this->dynamicStateEnables.data();
		this->dynamicStateInfo.dynamicStateCount = 
			static_cast <uint32_t> (this->dynamicStateEnables.size());
		this->dynamicStateInfo.flags = 0;
	}

	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo(PipelineConfigInfo&&) = default;
	PipelineConfigInfo& operator=(PipelineConfigInfo&&) = default;

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