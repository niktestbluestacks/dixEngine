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
- `m_sizeofPushConstantData`: Static constexpr size of the push constant data in bytes

Constructor parameters:
```cpp
DixRenderSystem(
    EngineDevice& engineDevice,
    VkRenderPass renderPass,
    VkDescriptorSetLayout globalSetLayout,
    VkDescriptorSetLayout modelSetLayout,
    std::string vertShaderBinaryPath,
    std::string fragShaderBinaryPath,
    std::function<void(void*, GameObject&)> transformGameObject
);
```

Note: The push constant size is now determined by the static constexpr member `m_sizeofPushConstantData` rather than being passed as a constructor parameter.

#### 2. RenderSystemRegistery (Template Class)

Location: `VulcanEngine/Rendering/RenderSystem/RenderSystemRegistery.hpp`

A template-based registry that manages multiple render systems at compile-time:

```cpp
template <typename... RenderSystems>
class RenderSystemRegistery;
```

The registry requires each render system to satisfy the following concepts (defined in `Utils/DixConcepts.hpp`):
- `HasUbos`: Must define a `Ubos` type alias (typically a `std::tuple` of UBO structures)
- `is_tuple<Ubos>`: The `Ubos` type must be a `std::tuple`
- `HasName`: Must provide a static `Name()` method returning `const char*`
- `HasVulkanFlags`: Must provide a static `getVulkanFlags()` method returning a tuple of descriptor bindings

Key structure: `RenderSystemDescription<RenderSystem>`
```cpp
template <typename RenderSystem>
struct RenderSystemDescription {
    std::unique_ptr<RenderSystem> renderSystem;  // Pointer to the render system instance
    RenderSystem::Ubos Ubos;                      // UBO type information
    const char* renderSystemName = RenderSystem::Name();
};
```

Key methods:
- `getRenderSystemDescriptions()`: Returns a tuple of `RenderSystemDescription` for all registered systems

Usage in AppContext:
```cpp
RenderSystemRegistery<RenderSystems...> m_renderSystemRegistery;
```

Accessing render systems uses `std::apply` to iterate over the tuple:
```cpp
std::apply([&](auto&&... renderSystemDescs) {
    (([&](auto&& desc) {
        const auto& renderSystemName = desc.renderSystemName;
        const auto& renderSystem = desc.renderSystem;
        // ... use renderSystem
    }(renderSystemDescs)), ...);
}, m_renderSystemRegistery.getRenderSystemDescriptions());
```

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

Required static methods for registry compatibility:
```cpp
static constexpr const char* Name() {
    return "SimpleRenderSystem";
}

static constexpr std::tuple<VulkanRenderSystemFlagType, VulkanRenderSystemFlagType> getVulkanFlags() {
    return std::make_tuple(
        VulkanRenderSystemFlagType{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
        VulkanRenderSystemFlagType{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
    );
}
```

UBO type alias for registry:
```cpp
using Ubos = std::tuple<SimpleUbo>;
```

## Using Render Systems with AppContext

### AppContext Overview

Location: `Applications/FirstApp/AppContext.hpp`

AppContext is a **template class** that encapsulates all rendering infrastructure, parameterized by render system types:

```cpp
template <typename... RenderSystems>
class AppContext;
```

This design allows compile-time registration of render systems without runtime overhead. The render systems are declared via template parameters when instantiating AppContext, eliminating the need for explicit declaration calls.

### Initialization Sequence

AppContext initializes render systems in a specific order during `initialize()`:

1. `createDescriptorPool()` - Create descriptor pool
2. `createUBOs()` - Create uniform buffers for each render system
3. `createSystemSetLayouts()` - Create descriptor set layouts based on `getVulkanFlags()`
4. `createDescriptorSets()` - Allocate and write descriptor sets
5. `createModelDescriptorResources()` - Create per-model descriptor resources
6. `createRenderSystems()` - Instantiate render systems

Note: Unlike older designs, there is no explicit `declareRenderSystems()` call. Render systems are declared via template parameters when instantiating AppContext.

### Step-by-Step Implementation Guide

#### Step 1: Define AppContext with Render Systems

Declare AppContext with your render system types as template parameters:
```cpp
AppContext<SimpleRenderSystem> appContext(800, 600, "My Application");
```

#### Step 2: Create Descriptor Pool

Creates a pool that can allocate descriptor sets for all render systems.

#### Step 3: Create Uniform Buffers

Creates UBOs for each frame in flight for each render system. The size is determined by iterating over the tuple of UBO types defined in each render system's `Ubos` alias.

#### Step 4: Create Descriptor Set Layouts

Defines the layout of descriptor sets for each render system by calling `getVulkanFlags()` on each render system instance. The method returns a tuple of `VulkanRenderSystemFlagType` which are then unpacked to build the descriptor set layout.

```cpp
auto&& vulkanFlags = renderSystemDesc.renderSystem->getVulkanFlags();
auto builder = DixDescriptorSetLayout::Builder(m_dixDevice);
std::apply([&](auto&&... bindingTuples) {
    (std::apply([&](auto&&... args) {
        builder.addBinding(std::forward<decltype(args)>(args)...);
    }, bindingTuples), ...);
}, vulkanFlags);
m_systemSetLayouts[renderSystemDesc.renderSystemName] = builder.build();
```

#### Step 5: Create Descriptor Sets

Allocates and writes descriptor sets for each frame, including the default texture for models without textures.

#### Step 6: Create Model Descriptor Resources

Sets up per-model descriptor resources including the model set layout and descriptor pool.

#### Step 7: Instantiate Render Systems

Creates render system instances using the constructor parameters. The implementation uses index sequence expansion to properly instantiate each render system:

```cpp
template<size_t... Indices>
void createRenderSystemsImpl(std::index_sequence<Indices...>) {
    (([&]() {
        using T = std::remove_reference_t<decltype(*std::get<Indices>(m_renderSystemRegistery.getRenderSystemDescriptions()).renderSystem)>;
        std::get<Indices>(m_renderSystemRegistery.getRenderSystemDescriptions()).renderSystem = std::make_unique<T>(
            m_dixDevice,
            m_dixRenderer.getSwapChainRenderPass(),
            m_systemSetLayouts[std::get<Indices>(m_renderSystemRegistery.getRenderSystemDescriptions()).renderSystemName]->getDescriptorSetLayout(),
            m_modelSetLayout->getDescriptorSetLayout()
        );
    })(), ...);
}
```

## Creating Custom Render Systems

### Step 1: Define UBO Structure

Create a UBO structure with proper alignment (must be multiple of 16 bytes):

```cpp
struct MyUbo {
    alignas(16) glm::mat4 projectionView{ 1.f };
    // Add other uniforms as needed
};
```

### Step 2: Create Render System Class

Inherit from `DixRenderSystem` and implement the required static methods and type aliases:

```cpp
class MyRenderSystem : public DixRenderSystem {
public:
    using DixRenderSystem::DixRenderSystem;  // inherit constructors
    using Ubos = std::tuple<MyUbo>;           // Required by registry

    MyRenderSystem(
        EngineDevice& engineDevice,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout,
        VkDescriptorSetLayout modelSetLayout
    );

    // Required static methods
    static constexpr const char* Name() {
        return "MyRenderSystem";
    }

    static constexpr auto getVulkanFlags() {
        return std::make_tuple(
            VulkanRenderSystemFlagType{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
            // Add more bindings as needed
        );
    }
};
```

### Step 3: Use in AppContext

Add the custom render system as a template parameter to AppContext:

```cpp
AppContext<MyRenderSystem> appContext(800, 600, "My Application");
```

### Step 4: Load Game Objects

Assign game objects to appropriate render systems using the render system name as the key.

## Technical Details

### Descriptor Set Layout

The descriptor set layout is defined by each render system's `getVulkanFlags()` method, which returns a tuple of `VulkanRenderSystemFlagType`:

```cpp
using VulkanRenderSystemFlagType = typename std::tuple<uint32_t, VkDescriptorType, VkShaderStageFlags>;
```

Each tuple element specifies:
- Binding number (uint32_t)
- Descriptor type (e.g., `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER`)
- Shader stage flags (e.g., `VK_SHADER_STAGE_VERTEX_BIT`)

### Push Constants

Push constants provide fast access to frequently changing data:
- Size limit: Typically 128-256 bytes (check VkPhysicalDeviceLimits)
- Updated per-object during rendering
- Accessible in both vertex and fragment shaders
- Size determined by the static constexpr `m_sizeofPushConstantData` in DixRenderSystem

### Alignment Requirements

- UBO structures must be aligned to 16 bytes
- Use `alignas(16)` for struct members
- Total struct size should be a multiple of 16 bytes

### Multi-Pass Rendering

Multiple render systems can be used for different passes. Each render system is instantiated separately and can have its own pipeline configuration.

### Thread Safety

- RenderSystemRegistery is a template class instantiated within AppContext
- All render system operations should occur on the main thread
- Descriptor set updates must occur outside of active render passes
- UBO updates should be done before submitting command buffers

## Best Practices

1. **Use template parameters**: Declare render systems via AppContext template parameters at compile-time
2. **Respect alignment requirements**: Always align UBO structures to 16 bytes using `alignas(16)`
3. **Implement required interfaces**: Ensure custom render systems define `Ubos`, `Name()`, and `getVulkanFlags()`
4. **Separate concerns**: Keep application logic in your app, rendering details in AppContext
5. **Handle window resize**: Check for zero extent before rendering to avoid Vulkan errors
6. **Use default textures**: Always provide a fallback texture for models without UVs
7. **Profile performance**: Monitor rendering performance during development

## Troubleshooting

### Common Issues

1. **Validation layer errors about descriptor sets**
   - Ensure descriptor set layouts match between creation and usage
   - Check that descriptor pools have sufficient capacity
   - Verify `getVulkanFlags()` returns correct binding numbers and types

2. **Alignment assertion failures**
   - Verify UBO structures use `alignas(16)` for all members
   - Total struct size must be a multiple of 16 bytes

3. **Missing textures on models**
   - Ensure models have valid UV coordinates
   - Check that the default texture is created properly

4. **Pipeline creation failures**
   - Verify shader binary paths are correct
   - Check that render pass is compatible with pipeline configuration

5. **Swapchain recreation issues**
   - Handle window resize events properly
   - Recreate descriptor sets if swapchain images change

6. **Template instantiation errors**
   - Ensure render system satisfies all concept requirements (`HasUbos`, `HasName`, `HasVulkanFlags`)
   - Verify `Ubos` is a `std::tuple` type

7. **UBO size calculation issues**
   - The current implementation uses `sizeof(info.Ubos)` which returns the size of the tuple itself, not the sum of its elements
   - For multiple UBOs in a tuple, manually calculate the total size with proper alignment
   - Each UBO member must be aligned to 16 bytes using `alignas(16)`

8. **Multiple render systems limitation**
   - The current `createRenderSystems()` implementation has hardcoded references to `std::get<0>`
   - This limits the system to a single render system until refactored
   - When adding multiple render systems, ensure all tuple iterations use proper parameter pack expansion

1. `createDescriptorPool()` - Create descriptor pool
2. `createUBOs()` - Create uniform buffers for each render system
3. `createSystemSetLayouts()` - Create descriptor set layouts based on `getVulkanFlags()`
4. `createDescriptorSets()` - Allocate and write descriptor sets
5. `createModelDescriptorResources()` - Create per-model descriptor resources
6. `createRenderSystems()` - Instantiate render systems

Note: Unlike older designs, there is no explicit `declareRenderSystems()` call. Render systems are declared via template parameters when instantiating AppContext.

### Step-by-Step Implementation Guide

#### Step 1: Define AppContext with Render Systems

Declare AppContext with your render system types as template parameters:
```cpp
AppContext<SimpleRenderSystem> appContext(800, 600, "My Application");
```

#### Step 2: Create Descriptor Pool

Creates a pool that can allocate descriptor sets for all render systems.

#### Step 3: Create Uniform Buffers

Creates UBOs for each frame in flight for each render system. The size is determined by `sizeof(info.Ubos)` where `info.Ubos` is the tuple type containing all UBO structures for that render system.

**Note**: The current implementation uses `sizeof(info.Ubos)` directly on the tuple type. For proper UBO sizing, you may need to calculate the total size of all UBO structures within the tuple, ensuring proper alignment (multiples of 16 bytes).

#### Step 4: Create Descriptor Set Layouts

Defines the layout of descriptor sets for each render system by calling `getVulkanFlags()` on each render system **instance** (not type). The method returns a tuple of `VulkanRenderSystemFlagType` which are then unpacked to build the descriptor set layout.

```cpp
auto&& vulkanFlags = renderSystemDesc.renderSystem->getVulkanFlags();
std::apply([&](auto&&... bindingTuples) {
    (std::apply([&](auto&&... args) {
        builder.addBinding(std::forward<decltype(args)>(args)...);
    }, bindingTuples), ...);
}, vulkanFlags);
```

#### Step 5: Create Descriptor Sets

Allocates and writes descriptor sets for each frame, including the default texture for models without textures.

#### Step 6: Create Model Descriptor Resources

Sets up per-model descriptor resources including the model set layout and descriptor pool.

#### Step 7: Instantiate Render Systems

Creates render system instances using the constructor parameters. The current implementation uses `std::apply` to iterate over render system descriptions but has a limitation - it hardcodes access to the first element:

```cpp
void createRenderSystems() {
    std::apply([this](auto&& arg) {
        using T = std::remove_reference_t<decltype(*(std::get<0>(m_renderSystemRegistery.getRenderSystemDescriptions()).renderSystem))>;
        std::get<0>(m_renderSystemRegistery.getRenderSystemDescriptions()).renderSystem = new T (
            m_dixDevice,
            m_dixRenderer.getSwapChainRenderPass(),
            m_systemSetLayouts[std::get<0>(m_renderSystemRegistery.getRenderSystemDescriptions()).renderSystemName]->getDescriptorSetLayout(),
            m_modelSetLayout->getDescriptorSetLayout()
        );
    }, m_renderSystemRegistery.getRenderSystemDescriptions());
}
```

**Important Limitation**: The current implementation incorrectly uses `std::get<0>` inside the lambda instead of using the `arg` parameter directly. This should be refactored to:

```cpp
std::apply([this](auto&&... args) {
    (([&](auto&& arg) {
        using T = std::remove_reference_t<decltype(*arg.renderSystem)>;
        arg.renderSystem = new T(
            m_dixDevice,
            m_dixRenderer.getSwapChainRenderPass(),
            m_systemSetLayouts[arg.renderSystemName]->getDescriptorSetLayout(),
            m_modelSetLayout->getDescriptorSetLayout()
        );
    }(args)), ...);
}, m_renderSystemRegistery.getRenderSystemDescriptions());
```

Until this is fixed, the system effectively supports only a single render system.

## Creating Custom Render Systems

### Step 1: Define UBO Structure

Create a UBO structure with proper alignment (must be multiple of 16 bytes):
```cpp
struct CustomUbo {
    alignas(16) glm::mat4 projectionView{ 1.f };
    alignas(16) glm::vec4 customData{ 0.f };
};
```

### Step 2: Create Render System Class

Inherit from `DixRenderSystem` and implement required static methods. Note that `getVulkanFlags()` is currently called on the **instance** (not as a static method), so it should be a regular static constexpr method:

```cpp
class CustomRenderSystem : public DixRenderSystem {
public:
    using DixRenderSystem::DixRenderSystem;
    using Ubos = std::tuple<CustomUbo>;

    // Constructor matching base class signature
    CustomRenderSystem(
        EngineDevice& engineDevice,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout,
        VkDescriptorSetLayout modelSetLayout
    ) : DixRenderSystem(
        engineDevice,
        renderPass,
        globalSetLayout,
        modelSetLayout,
        "path/to/vert.spv",
        "path/to/frag.spv",
        sizeof(CustomPushConstantData),
        transformGameObjectCallback
    ) {}

    static constexpr const char* Name() {
        return "CustomRenderSystem";
    }

    static constexpr std::tuple<VulkanRenderSystemFlagType> getVulkanFlags() {
        return std::make_tuple(
            VulkanRenderSystemFlagType{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
        );
    }

    DIX_DISABLE_COPY(CustomRenderSystem)
};
```

**Important**: The constructor must match the base class `DixRenderSystem` signature exactly, as it's inherited via `using DixRenderSystem::DixRenderSystem;` or explicitly defined.

### Step 3: Add to AppContext Template

Include your custom render system in the AppContext template parameter list:
```cpp
AppContext<SimpleRenderSystem, CustomRenderSystem> appContext(...);
```

### Step 4: Load Game Objects

Assign game objects to appropriate render systems using the render system name as the key:
```cpp
std::unordered_map<std::string, std::vector<GameObject>> gameObjects;
gameObjects["SimpleRenderSystem"].push_back(obj1);
gameObjects["CustomRenderSystem"].push_back(obj2);
```

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
- Use `alignas(16)` for struct members
- Total struct size must be a multiple of 16 bytes

### Multi-Pass Rendering

Multiple render systems can be used for different passes. Each render system is processed in order during `drawFrame()` using `std::apply` on the tuple of render system descriptions:

```cpp
std::apply([&](auto&&... renderSystemDescs) {
    (([&](auto&& desc) {
        const auto& renderSystemName = desc.renderSystemName;
        const auto& renderSystem = desc.renderSystem;

        // Update UBO for this system
        std::apply([&](auto&& arg) {
            std::remove_reference_t<decltype(arg)> ubo{};
            ubo.projectionView = camera.getProjection() * camera.getView();
            m_systemUboBuffers[renderSystemName][frameIndex]->writeToIndex(&ubo, IndexOfWriteToIndex);
            ++IndexOfWriteToIndex;
            m_systemUboBuffers[renderSystemName][frameIndex]->flush();
        }, desc.Ubos);

        // Render geometry
        renderSystem->renderGameObjects(frameInfo, gameObjects[renderSystemName]);
    }(renderSystemDescs)), ...);
}, m_renderSystemRegistery.getRenderSystemDescriptions());
```

**Note**: The current UBO update logic in `drawFrame()` assumes a single UBO per render system and uses `desc.Ubos` directly. For render systems with multiple UBOs in the tuple, this logic needs to be extended to iterate over all UBOs properly.

### Thread Safety

- AppContext and RenderSystemRegistery should be accessed from the main thread
- Descriptor set updates must occur outside of active render passes
- UBO updates should be done before submitting command buffers

## Best Practices

1. **Use template parameters**: Declare all render systems as template parameters to AppContext at compile time
2. **Respect alignment requirements**: Always align UBO structures to 16 bytes
3. **Implement required static methods**: Every render system must provide `Name()` and `getVulkanFlags()`
4. **Separate concerns**: Keep application logic in FirstApp, rendering details in AppContext
5. **Handle window resize**: Check for zero extent before rendering
6. **Use default textures**: Always provide a fallback texture for models without UVs
7. **Profile performance**: Use the built-in FPS counter to monitor rendering performance

## Troubleshooting

### Common Issues

1. **Validation layer errors about descriptor sets**
   - Ensure descriptor set layouts match between creation and usage
   - Check that descriptor pools have sufficient capacity

2. **Alignment assertion failures**
   - Verify UBO structures use `alignas(16)` for all members
   - Total struct size must be a multiple of 16 bytes

3. **Missing textures on models**
   - Ensure models have valid UV coordinates
   - Check that the default texture is created properly

4. **Pipeline creation failures**
   - Verify shader binary paths are correct
   - Check that render pass is compatible with pipeline configuration

5. **Swapchain recreation issues**
   - Handle window resize events properly
   - Recreate descriptor sets if swapchain images change

6. **Template instantiation errors**
   - Ensure your render system satisfies all required concepts: `HasUbos`, `HasName`, `HasVulkanFlags`
   - Verify `Ubos` is a `std::tuple` type
   - Check that the constructor signature matches the base class `DixRenderSystem`

7. **UBO size calculation issues**
   - The current implementation uses `sizeof(info.Ubos)` which returns the size of the tuple itself, not the sum of its elements
   - For multiple UBOs in a tuple, manually calculate the total size with proper alignment
   - Each UBO member must be aligned to 16 bytes using `alignas(16)`

8. **Multiple render systems limitation**
   - The current `createRenderSystems()` implementation has hardcoded references to `std::get<0>`
   - This limits the system to a single render system until refactored
   - When adding multiple render systems, ensure all tuple iterations use proper parameter pack expansion