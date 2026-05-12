#ifndef _FIRST_APP_HPP
#define _FIRST_APP_HPP
 
// dix
#include <Model/GameObject/GameObject.hpp>
#include <FirstApp/AppContext.hpp>
#include <Utils/Converter.hpp>
#include <Sound/DixAudio.hpp>

// std
#include <string>
#include <vector>
#include <string_view>

namespace dix {
class FirstApp {
private:
	void loadGameObjects(void);
	void loadUIElements(void);
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
	static constexpr std::string_view MODEL_FILEPATH_RELATIVE = "Applications/models";


	FirstApp(void);
	~FirstApp(void);

	FirstApp(const FirstApp&) = delete;
	FirstApp& operator=(const FirstApp&) = delete;

	void run(void);

private:
    // application context encapsulates renderer/device/shader details
	std::unique_ptr <AppContext> m_context{ std::make_unique<AppContext>(
		WIDTH, 
		HEIGHT, 
		static_cast<std::string>("First Application")
	) };
	std::unordered_map<std::string, std::vector<GameObject>> m_gameObjects;
	std::unordered_map<std::string, DixAudio> m_sounds;
	glm::vec3 playerPosition{ -1.f, -2.f, 2.f };
};
}	// namespace dix

#endif // _FIRST_APP_HPP