#ifndef DIX_RENDER_SYSTEM_HPP
#define DIX_RENDER_SYSTEM_HPP

// dix
#include <DixCamera/DixCamera.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Model/GameObject/GameObject.hpp>
#include <Pipeline/Pipeline/Pipeline.hpp>
#include <Utils/FrameInfo.hpp>
#include <Utils/Class.hpp>

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

	void setDescriptorPool(std::unique_ptr<DixDescriptorPool> pool) {
		m_descriptorPool = std::move(pool);
	}

	DixDescriptorPool& getDescriptorPool() {
		return *m_descriptorPool;
	}

	const DixDescriptorPool& getDescriptorPool() const {
		return *m_descriptorPool;
	}
protected:
    std::string m_vertShaderBinaryPath;
    std::string m_fragShaderBinaryPath;

	EngineDevice& m_dixDevice;
    std::unique_ptr<Pipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
    std::function<void(void*, GameObject&)> m_transformGameObject;
    static constexpr int m_sizeofPushConstantData = sizeof(PushConstantData);
	std::unique_ptr<DixDescriptorPool> m_descriptorPool;
};
}	// namespace dix
#endif // DIX_RENDER_SYSTEM_HPP