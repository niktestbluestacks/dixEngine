#ifndef WINDOW_CLASS_HPP
#define WINDOW_CLASS_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <cstdint>

namespace dix {

class Window {
private:
	void initWindow(void);

public:
	Window(int width, int height, std::string title);
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	~Window(void);

	bool shouldClose(void) const;
	VkExtent2D getExtent() const;

	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const;

private:
	const int m_width;
	const int m_height;

	std::string m_title;
	GLFWwindow* m_window;
}; // class Window
} // namespace dix

#endif // WINDOW_CLASS_HPP