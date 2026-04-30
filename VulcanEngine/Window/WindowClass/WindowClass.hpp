#ifndef WINDOW_CLASS_HPP
#define WINDOW_CLASS_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace dix {

class Window {
private:
	void initWindow(void);
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
public:
	Window(int width, int height, std::string title);
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	~Window(void);

	bool shouldClose(void) const;
	VkExtent2D getExtent(void) const;
	bool wasWindowResized(void) const;
	void resetWindowResizedFlag(void);
	GLFWwindow* getGLFWwindow(void) const;

	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const;

private:
	int m_width;
	int m_height;
	bool m_framebufferResized = false;

	std::string m_title;
	GLFWwindow* m_window;
}; // class Window
} // namespace dix

#endif // WINDOW_CLASS_HPP