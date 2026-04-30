#ifndef SIMPLE_RENDER_SYSTEM_HPP
#define SIMPLE_RENDER_SYSTEM_HPP

// dix
#include <DixCamera/DixCamera.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Model/GameObject/GameObject.hpp>
#include <Pipeline/Pipeline/Pipeline.hpp>
#include <Utils/FrameInfo.hpp>

// std
#include <memory>
#include <vector>

namespace dix {
class SimpleRenderSystem {
private:
	void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void createPipeline(VkRenderPass renderPass);
public:

	SimpleRenderSystem(
		EngineDevice& engineDeivce, 
		VkRenderPass renderPass, 
		VkDescriptorSetLayout globalSetLayout
	);
	~SimpleRenderSystem(void);

	SimpleRenderSystem(const SimpleRenderSystem&) = delete;
	SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

	void renderGameObjects(
		FrameInfo& frameInfo, 
		std::vector <GameObject>& gameObjects
	);

private:
	EngineDevice& m_dixDevice;

    std::unique_ptr<Pipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
};
}

#endif // SIMPLE_RENDER_SYSTEM_HPP