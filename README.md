# VulcanEngine Documentation

## Overview
VulcanEngine is a Vulkan-based 3D rendering engine with UI support, built with C++26. It provides a comprehensive rendering pipeline including device management, swap chain handling, graphics pipelines, model loading, and user interface rendering.

## Project Structure

```
/workspace/
├── VulcanEngine/           # Main engine source code
│   ├── Pipeline/           # Vulkan pipeline components
│   │   ├── EngineDevice/   # Device initialization and management
│   │   ├── SwapChain/      # Swap chain and framebuffer management
│   │   ├── Pipeline/       # Graphics pipeline creation
│   │   ├── ShaderModule/   # Shader module handling
│   │   ├── Buffer/         # Vulkan buffer management
│   │   └── DixDescriptors/ # Descriptor set management
│   ├── Rendering/          # High-level rendering systems
│   │   ├── Renderer/       # Main renderer orchestration
│   │   └── RenderSystem/   # Render system implementations
│   ├── Model/              # 3D model and texture handling
│   │   ├── GameObject/     # Game object representation
│   │   ├── DixTexture/     # Texture management
│   │   └── DixImage/       # Image handling
│   ├── UI/                 # User interface system
│   ├── DixCamera/          # Camera systems
│   ├── Input/              # Input handling
│   │   └── Keyboard/       # Keyboard input
│   ├── Window/             # Window management
│   │   └── WindowClass/    # GLFW window wrapper
│   ├── Logger/             # Logging utilities
│   ├── DixUI/              # Pre-built UI components
│   └── Utils/              # Utility functions and helpers
├── Shaders/                # GLSL shaders
│   ├── SimpleShader/       # 3D rendering shaders
│   └── UI/                 # UI rendering shaders
├── Applications/           # Example applications
├── Dependencies/           # External libraries
└── documentation/          # This documentation
```

## Core Components

### 1. Pipeline Layer (Low-Level Vulkan)

#### EngineDevice
- **File**: `documentation/pipeline/EngineDevice.md`
- Manages Vulkan instance, physical/logical devices, queues, and command pools
- Handles device initialization and resource allocation
- Provides helper functions for buffer and image creation

#### Window
- **File**: `documentation/pipeline/WindowClass.md`
- GLFW window wrapper for Vulkan rendering
- Handles window creation, resize events, and surface creation
- Provides framebuffer extent and resize tracking

#### SwapChain
- **File**: `documentation/pipeline/SwapChain.md`
- Manages swap chain creation and recreation
- Handles image views, depth buffers, and framebuffers
- Implements synchronization with semaphores and fences
- Supports seamless resize handling

#### Pipeline
- **File**: `documentation/pipeline/Pipeline.md`
- Creates and manages Vulkan graphics pipelines
- Loads shader modules from SPIR-V files
- Configures pipeline state (rasterization, blending, depth testing)
- Provides default configuration for 3D rendering

### 2. Shaders

#### Simple Shader (3D Rendering)
- **Vertex Shader**: Transforms vertices, calculates lighting
- **Fragment Shader**: Samples textures, applies vertex colors
- Features: Directional lighting, texture mapping, push constants for transforms

#### UI Shader (2D Interface)
- **Vertex Shader**: Converts pixel coordinates to NDC
- **Fragment Shader**: Samples font/UI textures
- Features: Pixel-perfect UI positioning, texture atlas support

See `documentation/shaders/shaders.md` for complete shader documentation.

## Namespace

All engine classes are in the `dix` namespace:
```cpp
#include <Pipeline/EngineDevice/EngineDevice.hpp>

dix::Window window(800, 600, "My App");
dix::EngineDevice device(window);
```

## Typical Initialization Flow

```cpp
// 1. Create window
dix::Window window(width, height, "VulcanEngine Application");

// 2. Initialize device
dix::EngineDevice device(window);

// 3. Create swap chain
auto swapChain = std::make_unique<dix::SwapChain>(device, window.getExtent());

// 4. Create pipeline layout and descriptors
// (Descriptor sets, pipeline layout creation)

// 5. Create graphics pipeline
dix::PipelineConfigInfo configInfo{};
dix::Pipeline::defaultPipelineConfigInfo(configInfo);
configInfo.renderPass = swapChain->getRenderPass();
configInfo.pipelineLayout = pipelineLayout;

auto pipeline = std::make_unique<dix::Pipeline>(
    device,
    "shaders/simple_shader.vert.spv",
    "shaders/simple_shader.frag.spv",
    configInfo);

// 6. Create render system
// (Application-specific render system)

// 7. Main loop
while (!window.shouldClose()) {
    // Handle events
    glfwPollEvents();

    // Check for resize
    if (window.wasWindowResized()) {
        // Recreate swap chain
        swapChain = std::make_unique<dix::SwapChain>(
            device,
            window.getExtent(),
            std::move(swapChain)
        );

        // Recreate pipeline if needed
        // Update viewport/scissor

        window.resetWindowResizedFlag();
    }

    // Acquire next image
    uint32_t imageIndex;
    VkResult result = swapChain->acquireNextImage(&imageIndex);

    // Handle swap chain recreation
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        continue;
    }

    // Record commands
    VkCommandBuffer commandBuffer = commandBuffers[imageIndex];
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // Begin render pass
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind pipeline and draw
    pipeline->bind(commandBuffer);
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);

    // End render pass
    vkCmdEndRenderPass(commandBuffer);

    vkEndCommandBuffer(commandBuffer);

    // Submit and present
    result = swapChain->submitCommandBuffers(&commandBuffer, &imageIndex);
}

// 8. Wait for device idle and cleanup
vkDeviceWaitIdle(device.device());
// Resources automatically cleaned up by destructors
```

## Key Design Patterns

### RAII Resource Management
All engine classes follow RAII principles:
- Resources acquired in constructors
- Resources released in destructors
- No manual cleanup required

### Non-Copyable Objects
Vulkan objects are non-copyable to prevent resource conflicts:
```cpp
EngineDevice(const EngineDevice&) = delete;
SwapChain(const SwapChain&) = delete;
Pipeline(const Pipeline&) = delete;
```

### Shared Ownership for Resize
Swap chains use `std::shared_ptr` for smooth resize transitions:
```cpp
SwapChain(EngineDevice& device, VkExtent2D extent, std::shared_ptr<SwapChain> previous);
```

## Configuration Options

### Debug Mode
Validation layers automatically enabled in debug builds:
```cpp
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
```

### Frame Latency
Configured in SwapChain:
```cpp
static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
```

## Supported Features

### Rendering
3D triangle rendering
Texture mapping
Directional lighting (Gouraud shading)
Depth buffering
Backface culling
Viewport and projection transforms

### User Interface
2D UI rendering
Font atlas text rendering
Pixel-perfect positioning
Texture-based UI elements

### Input
Keyboard input handling
Window resize handling

### Utilities
Logging system
FPS counter
Object ID management
Transform hierarchy

## Build Requirements

### Dependencies
- **Vulkan SDK**: Required for Vulkan API
- **GLFW**: Window management
- **GLM**: Mathematics library
- **tinyobjloader**: Model loading (if used)
- **stb_image**: Image loading (if used)

### Compiler
- C++26 or later
- Vulkan headers and libraries

### Build System
- CMake (CMakeLists.txt provided)

## Common Workflows

### Adding a New Shader
1. Write GLSL shader in `Shaders/` directory
2. Compile to SPIR-V: `glslc shader.vert -o shader.vert.spv`
3. Load in pipeline constructor:
   ```cpp
   auto pipeline = std::make_unique<Pipeline>(
       device,
       "shaders/new_shader.vert.spv",
       "shaders/new_shader.frag.spv",
       configInfo);
   ```

### Creating Custom Render System
1. Inherit from or create new render system class
2. Implement scene traversal
3. Set up descriptor sets per material/object
4. Issue draw calls in render pass

### Adding UI Elements
1. Create UI element class inheriting from `IUIElement`
2. Implement render method using UI pipeline
3. Register with `UIManager`
4. Update position/content as needed

## Performance Considerations

### Batching
- Group objects by material/pipeline
- Minimize pipeline binds
- Use instancing for repeated objects

### Memory
- Use appropriate memory types (device local vs host visible)
- Reuse resources when possible
- Clean up unused resources promptly

### Synchronization
- Use multiple frames in flight for CPU/GPU parallelism
- Minimize GPU-CPU sync points
- Use semaphores for queue synchronization

## Debugging

### Validation Layers
Enable in debug builds for error detection:
- Parameter validation
- Object lifetime tracking
- Thread safety checks
- Shader validation

### Debug Utils
- Object naming for identification
- Debug labels in command buffers
- Severity levels for messages

### Common Issues
1. **Validation Errors**: Read validation layer messages carefully
2. **Black Screen**: Check clear color, depth test, vertex winding
3. **Crashes**: Verify resource lifetimes and synchronization
4. **Memory Leaks**: Use validation layer memory tracking

## File Locations Summary

| Component | Header | Implementation | Docs |
|-----------|--------|----------------|------|
| EngineDevice | `Pipeline/EngineDevice/EngineDevice.hpp` | `Pipeline/EngineDevice/EngineDevice.cpp` | `documentation/pipeline/EngineDevice.md` |
| Window | `Window/WindowClass/WindowClass.hpp` | `Window/WindowClass/WindowClass.cpp` | `documentation/pipeline/WindowClass.md` |
| SwapChain | `Pipeline/SwapChain/SwapChain.hpp` | `Pipeline/SwapChain/SwapChain.cpp` | `documentation/pipeline/SwapChain.md` |
| Pipeline | `Pipeline/Pipeline/Pipeline.hpp` | `Pipeline/Pipeline/Pipeline.cpp` | `documentation/pipeline/Pipeline.md` |
| Shaders | `Shaders/` | N/A (GLSL) | `documentation/shaders/shaders.md` |

## Getting Started

1. **Build the engine**:
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

2. **Run an example application** from `Applications/`

3. **Read component documentation** in respective markdown files

4. **Modify shaders** in `Shaders/` and recompile

5. **Create your own application** using the engine components

## Contributing

When adding new features:
1. Follow existing code style
2. Add documentation in this folder
3. Update this README if necessary
4. Test with validation layers enabled

## License

[Add license information here]

## Acknowledgments

- Vulkan® is a registered trademark of The Khronos Group Inc.
- Built with Vulkan, GLFW, and GLM