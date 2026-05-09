# SwapChain Documentation

## Overview
The `SwapChain` class manages the Vulkan swap chain, which is responsible for presenting rendered images to the screen. It handles swap chain creation, image views, depth buffers, render passes, framebuffers, and synchronization primitives.

## File Location
- **Header**: `VulcanEngine/Pipeline/SwapChain/SwapChain.hpp`
- **Implementation**: `VulcanEngine/Pipeline/SwapChain/SwapChain.cpp`

## Class Structure

### Constants
```cpp
static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
```
Maximum number of frames that can be processed concurrently (frame latency).

### Public Members

#### Constructors & Destructor
```cpp
SwapChain(EngineDevice& deviceRef, VkExtent2D windowExtent);
SwapChain(EngineDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
~SwapChain();
```

**Constructor 1:**
- Creates a new swap chain from scratch
- **Parameters:**
  - `deviceRef`: Reference to EngineDevice
  - `windowExtent`: Target swap chain dimensions

**Constructor 2:**
- Creates a new swap chain while preserving resources from a previous one (used during resize)
- **Parameters:**
  - `deviceRef`: Reference to EngineDevice
  - `windowExtent`: New target dimensions
  - `previous`: Shared pointer to old swap chain for resource reuse

**Destructor:**
- Cleans up all swap chain resources including images, views, framebuffers, and sync objects

#### Deleted Copy Operations
```cpp
SwapChain(const SwapChain&) = delete;
SwapChain operator=(const SwapChain&) = delete;
```
Non-copyable to prevent resource management conflicts.

#### Accessor Methods
| Method | Return Type | Description |
|--------|-------------|-------------|
| `getFrameBuffer(index)` | `VkFramebuffer` | Get framebuffer at specified index |
| `getRenderPass()` | `VkRenderPass` | Get the render pass object |
| `getImageView(index)` | `VkImageView` | Get image view at specified index |
| `imageCount()` | `size_t` | Get number of swap chain images |
| `getSwapChainImageFormat()` | `VkFormat` | Get color image format |
| `getSwapChainExtent()` | `VkExtent2D` | Get swap chain dimensions |
| `width()` | `uint32_t` | Get swap chain width |
| `height()` | `uint32_t` | Get swap chain height |
| `extentAspectRatio()` | `float` | Get aspect ratio (width/height) |
| `findDepthFormat()` | `VkFormat` | Find suitable depth buffer format |

#### Core Methods
| Method | Return Type | Description |
|--------|-------------|-------------|
| `acquireNextImage(imageIndex)` | `VkResult` | Acquires next available swap chain image |
| `submitCommandBuffers(buffers, imageIndex)` | `VkResult` | Submits command buffers and presents image |
| `compareSwapFormats(swapChain)` | `bool` | Compares formats with another swap chain |

### Private Members

#### Initialization Methods
| Method | Description |
|--------|-------------|
| `init()` | Main initialization entry point |
| `createSwapChain()` | Creates the Vulkan swap chain |
| `createImageViews()` | Creates image views for swap chain images |
| `createDepthResources()` | Creates depth buffer images and views |
| `createRenderPass()` | Creates the render pass |
| `createFramebuffers()` | Creates framebuffers for each swap chain image |
| `createSyncObjects()` | Creates semaphores and fences for synchronization |

#### Helper Methods
| Method | Description |
|--------|-------------|
| `chooseSwapSurfaceFormat(formats)` | Selects best surface format from available options |
| `chooseSwapPresentMode(modes)` | Selects presentation mode (vsync, etc.) |
| `chooseSwapExtent(capabilities)` | Determines optimal swap chain resolution |

### Private Data Members

| Member | Type | Description |
|--------|------|-------------|
| `swapChainImageFormat` | `VkFormat` | Format of swap chain color images |
| `swapChainDepthFormat` | `VkFormat` | Format of depth buffer |
| `swapChainExtent` | `VkExtent2D` | Dimensions of swap chain |
| `swapChainFramebuffers` | `std::vector<VkFramebuffer>` | Framebuffers for each image |
| `renderPass` | `VkRenderPass` | Render pass object |
| `depthImages` | `std::vector<VkImage>` | Depth buffer images |
| `depthImageMemorys` | `std::vector<VkDeviceMemory>` | Depth buffer memory allocations |
| `depthImageViews` | `std::vector<VkImageView>` | Depth buffer image views |
| `swapChainImages` | `std::vector<VkImage>` | Swap chain image handles |
| `swapChainImageViews` | `std::vector<VkImageView>` | Views for swap chain images |
| `device` | `EngineDevice&` | Reference to engine device |
| `windowExtent` | `VkExtent2D` | Target window extent |
| `swapChain` | `VkSwapchainKHR` | Swap chain handle |
| `oldSwapChain` | `std::shared_ptr<SwapChain>` | Previous swap chain (for resize) |
| `imageAvailableSemaphores` | `std::vector<VkSemaphore>` | Signal when image is available |
| `renderFinishedSemaphores` | `std::vector<VkSemaphore>` | Signal when rendering is complete |
| `inFlightFences` | `std::vector<VkFence>` | Track frame submission status |
| `imagesInFlight` | `std::vector<VkFence>` | Track which fence is waiting for each image |
| `currentFrame` | `size_t` | Current frame index (for round-robin) |

## Detailed Method Descriptions

### acquireNextImage()
```cpp
VkResult acquireNextImage(uint32_t* imageIndex);
```
Acquires the next available swap chain image for rendering.

**Operations:**
1. Waits for the fence of the current frame to ensure previous frame is complete
2. Calls `vkAcquireNextImageKHR` to get the next image index
3. Handles suboptimal/outdated results
4. Returns result code

**Return Values:**
- `VK_SUCCESS`: Successfully acquired image
- `VK_SUBOPTIMAL_KHR`: Swap chain still works but not optimal (surface changed)
- `VK_ERROR_OUT_OF_DATE_KHR`: Swap chain incompatible with surface (needs recreation)

### submitCommandBuffers()
```cpp
VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);
```
Submits rendering commands and presents the image to the screen.

**Operations:**
1. Sets up semaphore wait/signal information
2. Configures command buffer submission info
3. Submits to graphics queue
4. Signals fence for completion tracking
5. Presents image to screen via `vkQueuePresentKHR`
6. Updates frame index for next frame

### chooseSwapSurfaceFormat()
```cpp
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
```
Selects the preferred surface format.

**Priority:**
1. Prefers `VK_FORMAT_B8G8R8A8_SRGB` with `VK_COLOR_SPACE_SRGB_NONLINEAR_KHR`
2. Falls back to first available format if preferred not found

### chooseSwapPresentMode()
```cpp
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
```
Selects presentation mode.

**Priority:**
1. `VK_PRESENT_MODE_MAILBOX_KHR` (triple buffering with vsync, no tearing)
2. Falls back to `VK_PRESENT_MODE_FIFO_KHR` (standard vsync)

### chooseSwapExtent()
```cpp
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
```
Determines swap chain resolution.

**Logic:**
- If `currentWidth` is `UINT32_MAX`, uses window extent (windowed mode)
- Otherwise, uses `currentWidth`/`currentHeight` (fullscreen mode)

## Usage Example
```cpp
// Create initial swap chain
auto swapChain = std::make_unique<dix::SwapChain>(device, window.getExtent());

// Main rendering loop
uint32_t imageIndex;
while (!window.shouldClose()) {
    // Acquire next image
    VkResult result = swapChain->acquireNextImage(&imageIndex);

    // Handle swap chain recreation if needed
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        swapChain = std::make_unique<dix::SwapChain>(device, window.getExtent(),
                                                      std::move(swapChain));
        continue;
    }

    // Record commands to commandBuffers[imageIndex]

    // Submit and present
    result = swapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

    // Handle recreation again
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        swapChain = std::make_unique<dix::SwapChain>(device, window.getExtent(),
                                                      std::move(swapChain));
        continue;
    }
}
```

## Synchronization Strategy
The swap chain uses a triple-buffering approach with these synchronization primitives:

1. **Image Available Semaphores**: Signaled when swap chain image is ready for rendering
2. **Render Finished Semaphores**: Signaled when fragment shader completes
3. **In-Flight Fences**: Track when GPU finishes processing a frame
4. **Images In Flight**: Tracks which fence is associated with each swap chain image

This allows CPU to stay ahead of GPU by `MAX_FRAMES_IN_FLIGHT` frames while preventing resource conflicts.

## Swap Chain Recreation Flow
When the window is resized:
1. Old swap chain becomes invalid
2. New swap chain constructor takes `oldSwapChain` parameter
3. Resources (like depth buffer) can be reused if formats match
4. Old swap chain is destroyed after new one is fully created
5. Framebuffers and image views are recreated with new dimensions

## Notes
- Uses double buffering (`MAX_FRAMES_IN_FLIGHT = 2`)
- Automatically handles window resize events
- Supports seamless swap chain recreation without visual artifacts
- Depth buffer format is chosen based on device capabilities
- Render pass includes both color and depth attachments
- Framebuffers are created per-swap-chain-image for parallel processing

## Dependencies
- **EngineDevice**: For device access and helper functions
- **Vulkan**: Core swap chain and synchronization APIs
- **C++ Standard Library**: `<vector>`, `<memory>`

## Common Issues
1. **Suboptimal Result**: Surface properties changed but swap chain still usable
2. **Out of Date**: Surface incompatible, must recreate swap chain immediately
3. **Minimized Window**: Swap chain recreation should be skipped when window is minimized
4. **Fullscreen Transitions**: May trigger automatic swap chain recreation