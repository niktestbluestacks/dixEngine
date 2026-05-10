# Render Systems Documentation

## Overview

The Render System architecture in the DixEngine provides a flexible and extensible framework for rendering game objects using Vulkan. This document covers the architecture, usage patterns with AppContext, and technical implementation details.

## Architecture

### Core Components

#### 1. DixRenderSystem (Base Class)

Location: `VulcanEngine/Rendering/RenderSystem/DixRenderSystem.hpp`

The base class for all render systems. It handles:
- Pipeline layout creation with descriptor sets and push constants
- Graphics pipeline creation with shader modules
- Rendering game objects with proper descriptor binding

Key members:
- `m_pipeline`: The graphics pipeline object
- `m_pipelineLayout`: Layout defining descriptor sets and push constants
- `m_dixDevice`: Reference to the EngineDevice for Vulkan operations
- `m_transformGameObject`: Callback function to transform game object data into push constant format

Constructor parameters:
```cpp
DixRenderSystem(
    EngineDevice& engineDevice,
    VkRenderPass renderPass,
    VkDescriptorSetLayout globalSetLayout,
    VkDescriptorSetLayout modelSetLayout,
    std::string vertShaderBinaryPath,
    std::string fragShaderBinaryPath,
    int sizeofPushConstantData,
    std::function<void(void*, GameObject&)> transformGameObject
);
```

#### 2. RenderSystemRegistery (Singleton)

A singleton registry that manages multiple render systems:

```cpp
#define DIX_RSR dix::RenderSystemRegistery::getInstance()
```

Key methods:
- `declareRenderSystem<UboStruct>(name)`: Declare a render system type before initialization
- `registerRenderSystem<SystemType, UboStruct>(name, constructInfo)`: Register and instantiate a render system
- `getRenderSystem(name)`: Retrieve a registered render system
- `getUboTypeInfo(name)`: Get UBO size and alignment information

#### 3. FrameInfo

Location: `VulcanEngine/Utils/FrameInfo.hpp`

Structure containing per-frame rendering data:
```cpp
struct FrameInfo {
    int frameIndex;                    // Current frame index (0 to MAX_FRAMES_IN_FLIGHT-1)
    float frameTime;                   // Time elapsed since last frame
    VkCommandBuffer commandBuffer;     // Command buffer for recording commands
    DixCamera& dixcamera;              // Camera reference for view/projection
    VkDescriptorSet globalDescriptorSet; // Global descriptor set for this frame
    VkExtent2D screenExtent;           // Current swapchain extent
};
```

#### 4. SimpleRenderSystem (Example Implementation)

Location: `VulcanEngine/Rendering/RenderSystem/SimpleRenderSystem/`

A concrete implementation demonstrating basic 3D object rendering with:
- Push constants for model matrix and normal matrix
- Global UBO for projection-view matrix and light direction
- Per-model descriptor sets for textures

UBO structure:
```cpp
struct SimpleUbo {
    alignas(16) glm::mat4 projectionView{ 1.f };
    alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
};
```

## Using Render Systems with AppContext

### AppContext Overview

Location: `Applications/FirstApp/AppContext.hpp`

AppContext encapsulates all rendering infrastructure, providing a clean interface for applications.

### Initialization Sequence

AppContext initializes render systems in a specific order during initialize():

1. declareRenderSystems() - Declare render system types
2. createDescriptorPool() - Create descriptor pool
3. createUBOs() - Create uniform buffers
4. createSystemSetLayouts() - Create descriptor set layouts
5. createDescriptorSets() - Allocate and write descriptor sets
6. createModelDescriptorResources() - Create per-model descriptor resources
7. createRenderSystem() - Register and instantiate render systems

### Step-by-Step Implementation Guide

#### Step 1: Declare Render Systems

Declare all render systems with their UBO types before any resources are created.

#### Step 2: Create Descriptor Pool

Create a pool that can allocate descriptor sets for all render systems.

#### Step 3: Create Uniform Buffers

Create UBOs for each frame in flight for each render system.

#### Step 4: Create Descriptor Set Layouts

Define the layout of descriptor sets for each render system.

#### Step 5: Create Descriptor Sets

Allocate and write descriptor sets for each frame.

#### Step 6: Create Model Descriptor Resources

Set up per-model descriptor resources.

#### Step 7: Register Render Systems

Finally, register and instantiate the render systems.

## Creating Custom Render Systems

### Step 1: Define UBO Structure

Create a UBO structure with proper alignment (must be multiple of 16 bytes).

### Step 2: Create Render System Class

Inherit from DixRenderSystem and implement the constructor.

### Step 3: Register in AppContext

Add the custom render system to AppContext by updating declareRenderSystems(), createSystemSetLayouts(), and createRenderSystem().

### Step 4: Load Game Objects

Assign game objects to appropriate render systems.

## Technical Details

### Descriptor Set Layout

The default descriptor set layout uses two sets:
- Set 0 (Global): Contains per-frame data like UBOs
- Set 1 (Per-model): Contains per-object data like textures

### Push Constants

Push constants provide fast access to frequently changing data:
- Size limit: Typically 128-256 bytes (check VkPhysicalDeviceLimits)
- Updated per-object during rendering
- Accessible in both vertex and fragment shaders

### Alignment Requirements

- UBO structures must be aligned to 16 bytes
- Use alignas(16) for struct members
- The registry asserts alignment at registration time

### Multi-Pass Rendering

Multiple render systems can be used for different passes.

### Thread Safety

- RenderSystemRegistery is a singleton and should be accessed from the main thread
- Descriptor set updates must occur outside of active render passes
- UBO updates should be done before submitting command buffers

## Best Practices

1. Declare all render systems early: Call declareRenderSystems() before creating any resources
2. Respect alignment requirements: Always align UBO structures to 16 bytes
3. Use the registry pattern: Access render systems through DIX_RSR macro
4. Separate concerns: Keep application logic in FirstApp, rendering details in AppContext
5. Handle window resize: Check for zero extent before rendering
6. Use default textures: Always provide a fallback texture for models without UVs
7. Profile performance: Use the built-in FPS counter to monitor rendering performance

## Troubleshooting

### Common Issues

1. Validation layer errors about descriptor sets
   - Ensure descriptor set layouts match between creation and usage
   - Check that descriptor pools have sufficient capacity

2. Alignment assertion failures
   - Verify UBO structures use alignas(16) for all members
   - Total struct size must be a multiple of 16 bytes

3. Missing textures on models
   - Ensure models have valid UV coordinates
   - Check that the default texture is created properly

4. Pipeline creation failures
   - Verify shader binary paths are correct
   - Check that render pass is compatible with pipeline configuration

5. Swapchain recreation issues
   - Handle window resize events properly
   - Recreate descriptor sets if swapchain images change