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

bool Window::isKeyPressed(int key) const {
    return ((glfwGetKey(m_window, key) == GLFW_PRESS) &&
            (m_currentKey == key || m_currentKey == -69420));
}

bool Window::isMouseButtonPressed(int key) const {
    return (glfwGetMouseButton(m_window, key));
}

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
    auto dixWindow =
        reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    auto it = dixWindow->m_keyBindings.find(key);
    if (it != dixWindow->m_keyBindings.end()) {
        switch (action) {
            case GLFW_PRESS: {
                if (it->second.callback) {
                    if (dixWindow->m_currentKey == -69420) {
                        it->second.callback();
                        if (it->second.overrideOtherCallbacks) {
                            dixWindow->m_currentKey = key;
                        }
                    } else if (dixWindow->m_currentKey == key) {
                        it->second.callback();
                        dixWindow->m_currentKey = -69420;
                    } else if (!GLFWIsLetterOrNumber(key)) {
                        it->second.callback();
                    }
                }
                break;
            }
            case GLFW_REPEAT: {
                if (it->second.callbackWhenHeld) {
                    if (dixWindow->m_currentKey == -69420) {
                        it->second.callbackWhenHeld();
                        if (it->second.overrideOtherCallbacks) {
                            dixWindow->m_currentKey = key;
                        }
                    } else if (dixWindow->m_currentKey == key) {
                        it->second.callbackWhenHeld();
                        dixWindow->m_currentKey = -69420;
                    } else if (!GLFWIsLetterOrNumber(key)) {
                        it->second.callbackWhenHeld();
                    }
                }
                break;
            }
            case GLFW_RELEASE: {
                if (it->second.callbackWhenRealeased) {
                    if (dixWindow->m_currentKey == -69420) {
                        it->second.callbackWhenRealeased();
                        if (it->second.overrideOtherCallbacks) {
                            dixWindow->m_currentKey = key;
                        }
                    } else if (dixWindow->m_currentKey == key) {
                        it->second.callbackWhenRealeased();
                        dixWindow->m_currentKey = -69420;
                    } else if (!GLFWIsLetterOrNumber(key)) {
                        it->second.callbackWhenRealeased();
                    }
                }
                break;
            }
        }
    }
}

void Window::charCallback(GLFWwindow* window, unsigned int codepoint) {
    auto dixWindow =
        reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
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

void Window::bindKey(int key, std::function<void()> callback,
                     bool overrideOtherCallbacks,
                     std::function<void()> callbackWhenHeld,
                     std::function<void()> callbackWhenRealeased) {
    m_keyBindings[key] = {callback, overrideOtherCallbacks, callbackWhenHeld,
                          callbackWhenRealeased};
}

void Window::unbindKey(int key) { m_keyBindings.erase(key); }

void Window::setCharCallback(std::function<void(char)> callback) {
    m_charCallback = callback;
}

void Window::setInputMode(int key, int mode) {
    glfwSetInputMode(m_window, key, mode);
}

void Window::getCursorPos(double* x, double* y) {
    glfwGetCursorPos(m_window, x, y);
}
}  // namespace dix
