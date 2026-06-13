#ifndef WINDOW_CLASS_HPP
#define WINDOW_CLASS_HPP

// libs
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stb_image.h>

// std
#include <cwctype>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace dix {

struct keyCallbacksType {
    std::function<void()> callback = nullptr;
    std::function<void()> callbackWhenHeld = nullptr;
    std::function<void()> callbackWhenRealeased = nullptr;
    bool overrideOtherCallbacks = false;
    bool bindedUnusually = false;
};

inline bool GLFWIsLetterOrNumber(int key) {
    return (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) ||
           (key >= GLFW_KEY_0 && key <= GLFW_KEY_9);
}

class Window {
   private:
    void initWindow(void);
    static void framebufferResizeCallback(GLFWwindow* window, int width,
                                          int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode,
                            int action, int modes);
    static void keyCallbackUnUsual(GLFWwindow* window, int key, int scancode,
                                   int action, int modes);
    static void charCallback(GLFWwindow* window, unsigned int codepoint);

   public:
    Window(int width, int height, std::string title);
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    ~Window(void);

    bool shouldClose(void) const;
    vk::Extent2D getExtent(void) const;
    bool wasWindowResized(void) const;
    bool isKeyPressedUsual(int key) const;
    bool isKeyPressedUnUsual(int key) const;
    bool isMouseButtonPressed(int key) const;
    const char* getClipboardText() const;
    void setClipboardText(const char* text) const;
    void resetWindowResizedFlag(void);
    GLFWwindow* getGLFWwindow(void) const;

    void createWindowSurface(vk::Instance instance,
                             vk::SurfaceKHR* surface) const;

    void setWindowIcon(const std::string& filepath);

    void bindKey(int key, std::function<void()> callback = nullptr,
                 bool overrideOtherCallbacks = false,
                 std::function<void()> callbackWhenHeld = nullptr,
                 std::function<void()> callbackWhenRealeased = nullptr);

    void bindKeyUnUsual(int key, std::function<void()> callback = nullptr,
                        std::function<void()> callbackWhenHeld = nullptr,
                        std::function<void()> callbackWhenRealeased = nullptr);

    void bindKey(int key, keyCallbacksType callbacks);
    void unbindKey(int key);
    void setCharCallback(std::function<void(char)> callback);
    void setInputMode(int key, int mode);
    void getCursorPos(double* x, double* y);
    void setWindowMode(bool fullscreen);
   private:
    int m_width;
    int m_height;
    bool m_framebufferResized = false;
    int m_currentKey = -69420;

    std::string m_title;
    GLFWwindow* m_window;
    GLFWmonitor* m_monitor;
    const GLFWvidmode* m_mode = nullptr;
    std::map<int, keyCallbacksType> m_keyBindings;
    std::function<void(char)> m_charCallback;
};  // class Window
}  // namespace dix

#endif  // WINDOW_CLASS_HPP