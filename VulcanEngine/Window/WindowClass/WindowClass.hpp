#ifndef WINDOW_CLASS_HPP
#define WINDOW_CLASS_HPP

// libs
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stb_image.h>

// std
#include <cwctype>
#include <string>
#include <map>
#include <functional>
#include <vector>

namespace dix {

inline bool GLFWIsLetterOrNumber(int key) {
    return (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) || (key >= GLFW_KEY_0 && key <= GLFW_KEY_9);
}

class Window {
   private:
    void initWindow(void);
    static void framebufferResizeCallback(GLFWwindow* window, int width,
                                          int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action,
                           int mods);
    static void charCallback(GLFWwindow* window, unsigned int codepoint);

   public:
    Window(int width, int height, std::string title);
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    ~Window(void);

    bool shouldClose(void) const;
    vk::Extent2D getExtent(void) const;
    bool wasWindowResized(void) const;
    void resetWindowResizedFlag(void);
    GLFWwindow* getGLFWwindow(void) const;

    void createWindowSurface(vk::Instance instance,
                             vk::SurfaceKHR* surface) const;

    void setWindowIcon(const std::string& filepath);

    void bindKey(int key, std::function<void()> callback, bool overrideOtherBidngs = false);
    void unbindKey(int key);
    void setCharCallback(std::function<void(char)> callback);

   private:
    int m_width;
    int m_height;
    bool m_framebufferResized = false;
    int m_currentKey = -69420;

    std::string m_title;
    GLFWwindow* m_window;
    std::map<int, std::pair<std::function<void()>, bool>> m_keyBindings;
    std::function<void(char)> m_charCallback;
};  // class Window
}   // namespace dix

#endif  // WINDOW_CLASS_HPP