#ifndef _FIRST_APP_HPP
#define _FIRST_APP_HPP

// dix
#include <Model/GameObject/GameObject.hpp>
#include <FirstApp/AppContext.hpp>

// std
#include <memory>
#include <vector>

namespace dix {
class FirstApp {
private:
	void loadGameObjects(void);
private:
	//void sierpinski(
	//	std::vector <Model::Vertex>& vertecies,
	//	int depth,
	//	glm::vec2 left,
	//	glm::vec2 right,
	//	glm::vec2 top,
	//	glm::vec3 leftColor,
	//	glm::vec3 rightColor,
	//	glm::vec3 topColor
	//);
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
    // application context encapsulates renderer/device/shader details
	AppContext m_context{ WIDTH, HEIGHT, static_cast<std::string>("Vulkan") };
	std::vector <GameObject> m_gameObjects;
};
}

#endif // _FIRST_APP_HPP