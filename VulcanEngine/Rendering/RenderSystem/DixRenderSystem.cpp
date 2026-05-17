// dix
#include <Rendering/RenderSystem/DixRenderSystem.hpp>
#include <Utils/Converter.hpp>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <cstdint>
#include <array>
#include <memory>
#include <cassert>

namespace dix {

struct SimplePushConstantData {
	glm::mat4 modelMatrix{ 1.f };
	glm::mat4 normalMatrix{ 1.f };
};

DixRenderSystem::DixRenderSystem(
		EngineDevice& engineDevice, 
		VkRenderPass renderPass, 
		VkDescriptorSetLayout globalSetLayout,
		VkDescriptorSetLayout modelSetLayout,
        std::string vertShaderBinaryPath, 
        std::string fragShaderBinaryPath,
        std::function<void(void*, GameObject&)> transformGameObject):
		m_dixDevice{ engineDevice },
        m_vertShaderBinaryPath{ vertShaderBinaryPath },
        m_fragShaderBinaryPath{ fragShaderBinaryPath },
        m_transformGameObject{ transformGameObject } {}

DixRenderSystem::~DixRenderSystem() {
	vkDestroyPipelineLayout(m_dixDevice.device(), m_pipelineLayout, nullptr);
}

void DixRenderSystem::createPipelineLayout(
		VkDescriptorSetLayout globalSetLayout, 
		VkDescriptorSetLayout modelSetLayout
	) {

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = m_sizeofPushConstantData;

	std::vector <VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout, modelSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast <uint32_t> (descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if (vkCreatePipelineLayout(m_dixDevice.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create pipelineLayout");
	}
}

void DixRenderSystem::createPipeline(VkRenderPass renderPass) {
	assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
    PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);

	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = m_pipelineLayout;

	m_pipeline = std::make_unique<Pipeline>(
		m_dixDevice,
		// simple shaders are really simple XD
		toShaderPath(m_vertShaderBinaryPath),
		toShaderPath(m_fragShaderBinaryPath),
		pipelineConfig
	);
}

void DixRenderSystem::renderGameObjects(
		FrameInfo& frameInfo,
		std::vector <GameObject>& gameObjects
	) const {
	// bind pipeline
	m_pipeline->bind(frameInfo.commandBuffer);


	VkDescriptorImageInfo currentImageInfo{};
	currentImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	for (auto& obj : gameObjects) {
        std::array<std::byte, m_sizeofPushConstantData> buffer;
		void* push = buffer.data();
        m_transformGameObject(push, obj);

		vkCmdPushConstants(
			frameInfo.commandBuffer,
			m_pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			m_sizeofPushConstantData,
			push);

		// bind descriptor sets: set 0 = global UBO, set 1 = per-model texture
		std::array<VkDescriptorSet, 2> descriptorSets{ frameInfo.globalDescriptorSet, VK_NULL_HANDLE };
		if (obj.model) {
			descriptorSets[1] = obj.model->getDescriptorSet();
		}

		vkCmdBindDescriptorSets (
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelineLayout,
			0,
			static_cast<uint32_t>(descriptorSets.size()),
			descriptorSets.data(),
			0,
			nullptr
		);

		if (obj.model) {
			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}
}
}	// namespace dix