#include <FirstApp/FirstApp.hpp>

namespace dix {
	void FirstApp::run(void) {
		while (!m_window.shouldClose()) {
			glfwPollEvents();
		}
	}
}