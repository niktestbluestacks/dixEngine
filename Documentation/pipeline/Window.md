# WindowClass Documentation

## Overview
The `Window` class encapsulates GLFW window creation and management for the Vulkan application. It handles window initialization, resize events, and surface creation for Vulkan rendering.

## File Location
- **Header**: `VulcanEngine/Window/WindowClass/WindowClass.hpp`
- **Implementation**: `VulcanEngine/Window/WindowClass/WindowClass.cpp`

## Class Structure

### Public Members

#### Constructor & Destructor
```cpp
Window(int width, int height, std::string title);
~Window(void);
```
- **Constructor**: Creates a GLFW window with specified dimensions and title, initializes GLFW, and sets up framebuffer resize callback.
- **Destructor**: Cleans up the GLFW window and terminates GLFW.

#### Deleted Copy Operations
```cpp
Window(const Window&) = delete;
Window& operator=(const Window&) = delete;
```
The class is non-copyable to prevent multiple objects from managing the same GLFW window.

#### Window Management Methods
| Method | Return Type | Description |
|--------|-------------|-------------|
| `shouldClose()` | `bool` | Checks if the window should close (user clicked close button) |
| `getExtent()` | `VkExtent2D` | Returns the current framebuffer extent (width/height as VkExtent2D) |
| `wasWindowResized()` | `bool` | Checks if the window was resized since last frame |
| `resetWindowResizedFlag()` | `void` | Resets the resize flag after handling resize event |
| `getGLFWwindow()` | `GLFWwindow*` | Returns the raw GLFW window pointer |
| `createWindowSurface()` | `void` | Creates a Vulkan surface for the window |

### Private Members

#### Private Methods
| Method | Description |
|--------|-------------|
| `initWindow()` | Initializes GLFW and creates the window with proper settings |
| `framebufferResizeCallback()` | Static callback invoked when framebuffer is resized |

#### Private Data Members
| Member | Type | Description |
|--------|------|-------------|
| `m_width` | `int` | Current window width in pixels |
| `m_height` | `int` | Current window height in pixels |
| `m_framebufferResized` | `bool` | Flag indicating window was resized |
| `m_title` | `std::string` | Window title |
| `m_window` | `GLFWwindow*` | GLFW window pointer |

## Detailed Method Descriptions

### Constructor
```cpp
Window(int width, int height, std::string title);
```
**Parameters:**
- `width`: Initial window width in pixels
- `height`: Initial window height in pixels
- `title`: Window title string

**Operations:**
1. Initializes GLFW library
2. Sets window hints (no OpenGL context, framebuffer transparency disabled)
3. Creates the GLFW window
4. Sets framebuffer resize callback
5. Stores initial dimensions

### createWindowSurface()
```cpp
void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const;
```
**Parameters:**
- `instance`: Vulkan instance handle
- `surface`: Pointer to VkSurfaceKHR to store the created surface

**Description:**
Creates a Vulkan surface for the window using GLFW's `glfwCreateWindowSurface`. This surface is required for presenting rendered images to the window.

### framebufferResizeCallback()
```cpp
static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
```
**Parameters:**
- `window`: GLFW window pointer (used to retrieve Window object via user pointer)
- `width`: New framebuffer width
- `height`: New framebuffer height

**Description:**
Static callback function invoked by GLFW when the framebuffer is resized. Updates the window dimensions and sets the resize flag.

## Usage Example
```cpp
// Create a window
dix::Window window(800, 600, "My Vulkan Application");

// Main loop
while (!window.shouldClose()) {
    // Check for resize events
    if (window.wasWindowResized()) {
        // Handle resize (recreate swap chain, etc.)
        window.resetWindowResizedFlag();
    }

    // Get current extent for swap chain recreation
    VkExtent2D extent = window.getExtent();

    // Render frame...
}

// Window destructor automatically cleans up GLFW resources
```

## Integration with EngineDevice
The Window class works closely with `EngineDevice`:
```cpp
dix::Window window(800, 600, "Vulkan App");
dix::EngineDevice device(window);  // Device creates surface using window

// Later, when creating swap chain:
VkSurfaceKHR surface;
window.createWindowSurface(device.instance(), &surface);
```

## Notes
- The class uses GLFW for window management with Vulkan integration (`GLFW_INCLUDE_VULKAN`)
- Framebuffer resize events are tracked for proper swap chain recreation
- No OpenGL context is created (GLFW configured for Vulkan only)
- The resize flag must be manually reset after handling the resize event
- Raw GLFW window pointer is accessible for advanced operations

## Dependencies
- **GLFW**: Window creation and event handling
- **Vulkan**: VkExtent2D, VkInstance, VkSurfaceKHR types
- **C++ Standard Library**: `<string>`

## Events Handled
1. **Window Creation**: Initializes GLFW and creates window
2. **Framebuffer Resize**: Tracks resize events for swap chain recreation
3. **Window Close**: Provides mechanism to check if user requested close
4. **Surface Creation**: Creates Vulkan surface for presentation