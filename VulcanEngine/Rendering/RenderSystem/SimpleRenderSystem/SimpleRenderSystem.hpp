#ifndef SIMPLE_RENDER_SYSTEM_HPP
#define SIMPLE_RENDER_SYSTEM_HPP

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Model/GameObject/GameObject.hpp>
#include <Pipeline/Pipeline/Pipeline.hpp>

// std
#include <memory>
#include <vector>

namespace dix {
class SimpleRenderSystem {
private:
	void createPipelineLayout(void);
	void createPipeline(VkRenderPass renderPass);
public:

	SimpleRenderSystem(EngineDevice& engineDeivce, VkRenderPass renderPass);
	~SimpleRenderSystem(void);

	SimpleRenderSystem(const SimpleRenderSystem&) = delete;
	SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

	void renderGameObjects(VkCommandBuffer commandBuffer, std::vector <GameObject>& gameObjects);
private:
	EngineDevice& m_dixDevice;
	std::unique_ptr<Pipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
};
}

#endif // SIMPLE_RENDER_SYSTEM_HPP