#ifndef _FIRST_APP_HPP
#define _FIRST_APP_HPP

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>
#include <Model/GameObject/GameObject.hpp>
#include <Window/WindowClass/WindowClass.hpp>
#include <Rendering/Renderer/Renderer.hpp>
#include <Logger/Logger.hpp>

// std
#include <memory>
#include <vector>

namespace dix {
class FirstApp {
private:
	void loadGameObjects(void);
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
	static constexpr int WIDTH = 800;
	static constexpr int HEIGHT = 600;
	static constexpr float MAX_FRAME_TIME = 0.05f;

	FirstApp(void);
	~FirstApp(void);

	FirstApp(const FirstApp&) = delete;
	FirstApp& operator=(const FirstApp&) = delete;

	void run(void);

private:
	Window m_Window{ WIDTH, HEIGHT, static_cast <std::string> ("Vulkan") };
	EngineDevice m_dixDevice{ m_Window };
	Renderer m_dixRenderer{ m_Window, m_dixDevice };

	// note: order of allocation matters
	std::unique_ptr <DixDescriptorPool> globalPool{};
	std::vector <GameObject> m_gameObjects;
};
}

#endif // _FIRST_APP_HPP