# EngineDevice Documentation

## Overview
The `EngineDevice` class is the core Vulkan device management class responsible for initializing and managing the Vulkan instance, physical device, logical device, surface, and command pool. It encapsulates all low-level Vulkan device operations.

## File Location
- **Header**: `VulcanEngine/Pipeline/EngineDevice/EngineDevice.hpp`
- **Implementation**: `VulcanEngine/Pipeline/EngineDevice/EngineDevice.cpp`

## Class Structure

### Public Members

#### Constructor & Destructor
```cpp
EngineDevice(Window& window);
~EngineDevice();
```
- **Constructor**: Initializes the Vulkan instance, selects a physical device, creates a logical device, and sets up the command pool.
- **Destructor**: Cleans up all Vulkan resources in reverse order of creation.

#### Deleted Copy/Move Operations
```cpp
EngineDevice(const EngineDevice&) = delete;
EngineDevice& operator=(const EngineDevice&) = delete;
EngineDevice(EngineDevice&&) = delete;
EngineDevice& operator=(EngineDevice&&) = delete;
```
The class is non-copyable and non-movable to prevent resource management issues.

#### Accessor Methods
| Method | Return Type | Description |
|--------|-------------|-------------|
| `getCommandPool()` | `VkCommandPool` | Returns the command pool for allocating command buffers |
| `device()` | `VkDevice` | Returns the logical Vulkan device |
| `surface()` | `VkSurfaceKHR` | Returns the window surface for presentation |
| `graphicsQueue()` | `VkQueue` | Returns the graphics queue |
| `presentQueue()` | `VkQueue` | Returns the presentation queue |
| `getSwapChainSupport()` | `SwapChainSupportDetails` | Queries swap chain capabilities |
| `findMemoryType()` | `uint32_t` | Finds suitable memory type index |
| `findPhysicalQueueFamilies()` | `QueueFamilyIndices` | Gets queue family indices |
| `findSupportedFormat()` | `VkFormat` | Finds a supported image format |

#### Buffer Helper Functions
| Method | Description |
|--------|-------------|
| `createBuffer()` | Creates a Vulkan buffer with specified size, usage, and memory properties |
| `beginSingleTimeCommands()` | Begins a single-time command buffer for one-off operations |
| `endSingleTimeCommands()` | Ends and submits a single-time command buffer |
| `copyBuffer()` | Copies data from one buffer to another |
| `copyBufferToImage()` | Copies buffer data to an image |
| `createImageWithInfo()` | Creates an image with custom create info |
| `transitionImageLayout()` | Transitions an image to a different layout |

### Public Data Members
```cpp
const bool enableValidationLayers;  // Enabled in debug builds
VkPhysicalDeviceProperties properties;  // Physical device properties
```

### Private Methods (Initialization Helpers)
| Method | Description |
|--------|-------------|
| `createInstance()` | Creates the Vulkan instance |
| `setupDebugMessenger()` | Sets up debug messenger for validation layers |
| `createSurface()` | Creates a window surface via GLFW |
| `pickPhysicalDevice()` | Selects a suitable physical device (GPU) |
| `createLogicalDevice()` | Creates the logical device from the physical device |
| `createCommandPool()` | Creates the command pool for command buffer allocation |

### Private Helper Methods
| Method | Description |
|--------|-------------|
| `isDeviceSuitable()` | Checks if a physical device meets requirements |
| `getRequiredExtensions()` | Returns list of required Vulkan extensions |
| `checkValidationLayerSupport()` | Verifies validation layer availability |
| `findQueueFamilies()` | Finds graphics and present queue families |
| `populateDebugMessengerCreateInfo()` | Fills debug messenger create info struct |
| `hasGflwRequiredInstanceExtensions()` | Checks for required GLFW extensions |
| `checkDeviceExtensionSupport()` | Verifies device extension support |
| `querySwapChainSupport()` | Queries detailed swap chain capabilities |

### Private Data Members
| Member | Type | Description |
|--------|------|-------------|
| `instance` | `VkInstance` | Vulkan instance |
| `debugMessenger` | `VkDebugUtilsMessengerEXT` | Debug messenger handle |
| `physicalDevice` | `VkPhysicalDevice` | Selected physical device (GPU) |
| `window` | `Window&` | Reference to the window object |
| `commandPool` | `VkCommandPool` | Command pool for command buffers |
| `device_` | `VkDevice` | Logical device |
| `surface_` | `VkSurfaceKHR` | Window surface |
| `graphicsQueue_` | `VkQueue` | Graphics command queue |
| `presentQueue_` | `VkQueue` | Presentation queue |
| `validationLayers` | `std::vector<const char*>` | List of validation layers |
| `deviceExtensions` | `std::vector<const char*>` | Required device extensions |

## Supporting Structures

### SwapChainSupportDetails
```cpp
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
```
Holds all information needed to create a swap chain.

### QueueFamilyIndices
```cpp
struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool isComplete();
};
```
Stores indices of queue families that support graphics and presentation operations.

## Usage Example
```cpp
// Create window first
dix::Window window(width, height, "My Application");

// Create engine device
dix::EngineDevice device(window);

// Access the logical device
VkDevice logicalDevice = device.device();

// Get graphics queue
VkQueue graphicsQueue = device.graphicsQueue();

// Create a buffer using helper function
VkBuffer buffer;
VkDeviceMemory bufferMemory;
device.createBuffer(size, usage, properties, buffer, bufferMemory);
```

## Notes
- Validation layers are automatically enabled in debug builds (`#ifndef NDEBUG`)
- The class manages the entire Vulkan device lifecycle
- All resources are cleaned up in the destructor
- The class follows RAII principles for resource management

## Dependencies
- `WindowClass.hpp` - Requires a Window object for surface creation
- Vulkan SDK headers and libraries
- GLFW for window surface creation