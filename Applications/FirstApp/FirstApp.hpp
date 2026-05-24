#ifndef _FIRST_APP_HPP
#define _FIRST_APP_HPP
 
#include <FirstApp/AppContext.hpp>

namespace dix {

using CurrentAppContext = AppContext <
	SimpleRenderSystem, 
	ParticleRenderSystem,
	SkyboxRenderSystem
>;

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
	std::unique_ptr <CurrentAppContext> m_context{ 
		std::make_unique<CurrentAppContext>(
			WIDTH, 
			HEIGHT, 
			static_cast<std::string>("First Application")
	) };
};
}	// namespace dix

#endif // _FIRST_APP_HPP