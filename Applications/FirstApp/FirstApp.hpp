#ifndef _FIRST_APP_HPP
#define _FIRST_APP_HPP

#include <Window/WindowClass/WindowClass.hpp>
#include <Pipeline/Pipeline/Pipeline.hpp>
#include <Utils/Converter.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>

namespace dix {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		void run(void);

	private:
		Window m_window{ WIDTH, HEIGHT, static_cast <std::string> ("Vulkan") };
		EngineDevice m_dixdevice{ m_window };
		Pipeline m_pipeline{m_dixdevice,
			dix::toShaderPath("SimpleShader/simple_shader.vert.spv"),
			dix::toShaderPath("SimpleShader/simple_shader.frag.spv"),
			Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
	};
}

#endif // _FIRST_APP_HPP