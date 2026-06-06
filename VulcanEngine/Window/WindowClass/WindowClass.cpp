// dix
#include <Window/WindowClass/WindowClass.hpp>

// std
#include <cstdint>
#include <stdexcept>

namespace dix {

Window::Window(int width, int height, std::string title)
    : m_width(width), m_height(height), m_title(title) {
    initWindow();
}

Window::~Window(void) {
    glfwDestroyWindow(m_window);
    m_window = nullptr;
    glfwTerminate();
}

bool Window::shouldClose(void) const { return glfwWindowShouldClose(m_window); }

vk::Extent2D Window::getExtent(void) const {
    return vk::Extent2D{static_cast<uint32_t>(m_width),
                        static_cast<uint32_t>(m_height)};
}

bool Window::wasWindowResized(void) const { return m_framebufferResized; }

void Window::resetWindowResizedFlag(void) { m_framebufferResized = false; }

GLFWwindow* Window::getGLFWwindow(void) const { return m_window; }

void Window::createWindowSurface(vk::Instance instance,
                                 vk::SurfaceKHR* surface) const {
    if (glfwCreateWindowSurface(
            static_cast<VkInstance>(instance), m_window, nullptr,
            reinterpret_cast<VkSurfaceKHR*>(surface)) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void Window::initWindow(void) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window =
        glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetCharCallback(m_window, charCallback);
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width,
                                       int height) {
    auto dixWindow =
        reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    dixWindow->m_framebufferResized = true;
    dixWindow->m_width = width;
    dixWindow->m_height = height;
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action,
                        int mods) {
    if (action != GLFW_PRESS) {
        return;
    }

    auto dixWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    auto it = dixWindow->m_keyBindings.find(key);
    if (it != dixWindow->m_keyBindings.end()) {
        if (dixWindow->m_currentKey == -69420) {
            it->second.first();
            if (it->second.second) {
                dixWindow->m_currentKey = key;
            }
        } else if (dixWindow->m_currentKey == key) {
            it->second.first();
            dixWindow->m_currentKey = -69420;
        } else if (!GLFWIsLetterOrNumber(key)) {
            it->second.first();
        }
    }
}

void Window::charCallback(GLFWwindow* window, unsigned int codepoint) {
    auto dixWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    if (dixWindow->m_charCallback && codepoint < 128) {
        dixWindow->m_charCallback(static_cast<char>(codepoint));
    }
}

void Window::setWindowIcon(const std::string& filepath) {
    GLFWimage images[1];
    images[0].pixels =
        stbi_load(filepath.c_str(), &images[0].width, &images[0].height, 0, 4);
    if (images[0].pixels) {
        glfwSetWindowIcon(m_window, 1, images);
        stbi_image_free(images[0].pixels);
    }
}

void Window::bindKey(int key, std::function<void()> callback, bool overrideOtherBidngs) {
    m_keyBindings[key] = std::make_pair(callback, overrideOtherBidngs);
}

void Window::unbindKey(int key) {
    m_keyBindings.erase(key);
}

void Window::setCharCallback(std::function<void(char)> callback) {
    m_charCallback = callback;
}

}  // namespace dix
