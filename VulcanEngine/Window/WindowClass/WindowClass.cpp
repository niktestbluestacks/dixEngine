#include <Window/WindowClass/WindowClass.hpp>

#include <stdexcept>

namespace dix {
Window::Window(int width, int height, std::string title) :
	m_width(width),
	m_height(height),
	m_title(title) {

	initWindow();
}

Window::~Window(void) {
	glfwDestroyWindow(m_window);
	m_window = nullptr;
	glfwTerminate();
}

bool Window::shouldClose(void) const {
	return glfwWindowShouldClose(m_window);
}

VkExtent2D Window::getExtent() const {
	return VkExtent2D{ static_cast <uint32_t> (m_width), static_cast <uint32_t> (m_height) };
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const {
	if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

void Window::initWindow(void) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
}
}