#ifndef _FIRST_APP_HPP
#define _FIRST_APP_HPP

#include <Window/WindowClass/WindowClass.hpp>
#include <Pipeline/Pipeline/Pipeline.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/SwapChain/SwapChain.hpp>
#include <Logger/Logger.hpp>

// std
#include <memory>
#include <vector>

namespace dix {
	class FirstApp {
	private:
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		void operator=(const FirstApp&) = delete;

		void run(void);

	private:
		Window m_Window{ WIDTH, HEIGHT, static_cast <std::string> ("Vulkan") };
		EngineDevice m_dixDevice{ m_Window };
		SwapChain m_dixSwapChain{ m_dixDevice, m_Window.getExtent() };
		std::unique_ptr<Pipeline> m_pipeline;
		VkPipelineLayout m_pipelineLayout;
		std::vector<VkCommandBuffer> m_commandBuffers;
	};
}

#endif // _FIRST_APP_HPP