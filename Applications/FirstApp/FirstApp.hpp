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
public:
	static constexpr int WIDTH = 800;
	static constexpr int HEIGHT = 600;
	static constexpr float MAX_FRAME_TIME = 0.05f;
	static constexpr std::string_view MODEL_FILEPATH_RELATIVE = "Applications/models";


	FirstApp(void);
	~FirstApp(void);

	DIX_DISABLE_COPY(FirstApp)

	void run(void);

private:
	glm::vec3 playerPosition{ -1.f, -2.f, 2.f };
	glm::vec3 playerLookAt { 0.f, 0.f, 2.5f };
	std::unordered_map<std::string, std::vector<GameObject>> m_gameObjects;
	std::unordered_map<std::string, DixAudio> m_sounds;
	std::unique_ptr <AppContext<SimpleRenderSystem>> m_context{ std::make_unique<AppContext<SimpleRenderSystem>>(
		WIDTH, 
		HEIGHT, 
		static_cast<std::string>("First Application")
	) };
};
}	// namespace dix

#endif // _FIRST_APP_HPP