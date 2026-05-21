#ifndef DIX_RENDER_SYSTEM_HPP
#define DIX_RENDER_SYSTEM_HPP

// dix
#include <DixCamera/DixCamera.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Model/GameObject/GameObject.hpp>
#include <Pipeline/Pipeline/Pipeline.hpp>
#include <Utils/FrameInfo.hpp>
#include <Utils/Class.hpp>
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>
#include <Pipeline/SwapChain/SwapChain.hpp>

// std
#include <memory>
#include <vector>
#include <array>

namespace dix {

using VulkanRenderSystemFlagType = typename std::tuple<uint32_t, VkDescriptorType, VkShaderStageFlags>;

struct BasePushConstantData {
	glm::mat4 modelMatrix{ 1.f };
	glm::mat4 normalMatrix{ 1.f };
};

class DixRenderSystem {
protected:
    virtual void createPipelineLayout(
        VkDescriptorSetLayout globalSetLayout, 
        VkDescriptorSetLayout modelSetLayout
    );
	virtual void createPipeline(VkRenderPass renderPass);
public:
	using PushConstantData = BasePushConstantData;

	DixRenderSystem(
		EngineDevice& engineDevice, 
		VkRenderPass renderPass, 
		VkDescriptorSetLayout globalSetLayout,
		VkDescriptorSetLayout modelSetLayout,
		DixDescriptorPool& descriptorPool,
        std::string vertShaderBinaryPath, 
        std::string fragShaderBinaryPath,
        std::function<void(void*, GameObject&)> transformGameObject
	);
	virtual ~DixRenderSystem(void);

	DIX_DISABLE_COPY(DixRenderSystem)

	virtual void renderGameObjects(
		FrameInfo& frameInfo, 
		std::vector <GameObject>& gameObjects
	) const;
protected:
	void setupDescriptors();

    std::string m_vertShaderBinaryPath;
    std::string m_fragShaderBinaryPath;

	EngineDevice& m_dixDevice;
    std::unique_ptr<Pipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
    std::function<void(void*, GameObject&)> m_transformGameObject;
    static constexpr int m_sizeofPushConstantData = sizeof(PushConstantData);

	// Per-frame descriptor sets managed internally
	std::array<VkDescriptorSet, SwapChain::MAX_FRAMES_IN_FLIGHT> m_descriptorSets{};
	std::unique_ptr<DixDescriptorSetLayout> m_globalSetLayout;
	DixDescriptorPool& m_descriptorPool;
};
}	// namespace dix
#endif // DIX_RENDER_SYSTEM_HPP