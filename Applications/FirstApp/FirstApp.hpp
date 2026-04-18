#ifndef _FIRST_APP_HPP
#define _FIRST_APP_HPP

// dix
#include <Window/WindowClass/WindowClass.hpp>
#include <Pipeline/Pipeline/Pipeline.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/SwapChain/SwapChain.hpp>
#include <Logger/Logger.hpp>
#include <Model/Model.hpp>

// std
#include <memory>
#include <vector>

namespace dix {
class FirstApp {
private:
	void loadModels();
	void createPipelineLayout();
	void createPipeline();
	void createCommandBuffers();
	void freeCommandBuffers();
	void drawFrame();
	void recreateSwapChain();
	void recordCommandBuffer(int imageIndex);
private:
	void sierpinski(
		std::vector <Model::Vertex>& vertecies,
		int depth,
		glm::vec2 left,
		glm::vec2 right,
		glm::vec2 top,
		glm::vec3 leftColor,
		glm::vec3 rightColor,
		glm::vec3 topColor);
public:
	static constexpr int WIDTH = 1440;
	static constexpr int HEIGHT = 1080;

	FirstApp();
	~FirstApp();

	FirstApp(const FirstApp&) = delete;
	FirstApp& operator=(const FirstApp&) = delete;

	void run(void);

private:
	Window m_Window{ WIDTH, HEIGHT, static_cast <std::string> ("Vulkan") };
	EngineDevice m_dixDevice{ m_Window };
	std::unique_ptr <SwapChain> m_dixSwapChain;
	std::unique_ptr<Pipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
	std::vector<VkCommandBuffer> m_commandBuffers;
	std::unique_ptr <Model> m_dixModel;
};
}

#endif // _FIRST_APP_HPP