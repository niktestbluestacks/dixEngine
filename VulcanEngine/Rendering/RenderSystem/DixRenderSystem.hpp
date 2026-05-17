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
#include <concepts>

namespace dix {

using VulkanRenderSystemFlagType = typename std::tuple<uint32_t, VkDescriptorType, VkShaderStageFlags>;

class DixRenderSystem {
protected:
    virtual void createPipelineLayout(
        VkDescriptorSetLayout globalSetLayout, 
        VkDescriptorSetLayout modelSetLayout
    );
	virtual void createPipeline(VkRenderPass renderPass);
public:

	DixRenderSystem(
		EngineDevice& engineDeivce, 
		VkRenderPass renderPass, 
		VkDescriptorSetLayout globalSetLayout,
		VkDescriptorSetLayout modelSetLayout,
        std::string vertShaderBinaryPath, 
        std::string fragShaderBinaryPath,
        int sizeofPushConstantData,
        std::function<void(void*, GameObject&)> transformGameObject
	);
	virtual ~DixRenderSystem(void);

	DIX_DISABLE_COPY(DixRenderSystem)

	virtual void renderGameObjects(
		FrameInfo& frameInfo, 
		std::vector <GameObject>& gameObjects
	) const;
protected:
    std::string m_vertShaderBinaryPath;
    std::string m_fragShaderBinaryPath;

	EngineDevice& m_dixDevice;
    std::unique_ptr<Pipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
    std::function<void(void*, GameObject&)> m_transformGameObject;
    int m_sizeofPushConstantData;
};
}	// namespace dix
#endif // DIX_RENDER_SYSTEM_HPP