# Pipeline Documentation

## Overview
The `Pipeline` class encapsulates the Vulkan graphics pipeline, which defines how vertex data is processed and converted to pixels on the screen. It manages shader modules, pipeline creation, and binding operations.

## File Location
- **Header**: `VulcanEngine/Pipeline/Pipeline/Pipeline.hpp`
- **Implementation**: `VulcanEngine/Pipeline/Pipeline/Pipeline.cpp`

## Related Structures

### PipelineConfigInfo
A configuration structure that holds all the state needed to create a graphics pipeline.

```cpp
struct PipelineConfigInfo {
    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizetionInfo;
    VkPipelineMultisampleStateCreateInfo multisampleStateInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    uint32_t subpass;

    // Optional custom vertex input descriptions
    std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
};
```

**Key Fields:**
- `viewportInfo`: Defines viewport and scissor rectangles
- `inputAssemblyInfo`: Specifies how vertices are assembled (triangles, lines, etc.)
- `rasterizetionInfo`: Controls rasterization (polygon mode, culling, etc.)
- `multisampleStateInfo`: Configures anti-aliasing
- `colorBlendAttachment`: Per-attachment color blending settings
- `colorBlendInfo`: Overall color blending configuration
- `depthStencilInfo`: Depth and stencil testing configuration
- `dynamicStateInfo`: States that can be changed without recreating pipeline
- `pipelineLayout`: Descriptor set and push constant layouts
- `renderPass`: Compatible render pass
- `vertexBindingDescriptions`: Vertex buffer binding descriptions
- `vertexAttributeDescriptions`: Vertex attribute descriptions

## Class Structure

### Public Members

#### Constructor & Destructor
```cpp
Pipeline(
    EngineDevice& device,
    const std::string& vertFilepath,
    const std::string& fragFilepath,
    const PipelineConfigInfo& configInfo);
~Pipeline();
```

**Constructor Parameters:**
- `device`: Reference to EngineDevice for Vulkan device access
- `vertFilepath`: Path to vertex shader SPIR-V file
- `fragFilepath`: Path to fragment shader SPIR-V file
- `configInfo`: Pipeline configuration structure

**Operations:**
1. Reads vertex and fragment shader files
2. Creates shader modules from SPIR-V code
3. Creates the graphics pipeline using provided configuration

**Destructor:**
- Destroys shader modules
- Destroys the graphics pipeline

#### Deleted Copy Operations
```cpp
Pipeline(const Pipeline&) = delete;
Pipeline operator=(const Pipeline&) = delete;
```
Non-copyable to prevent resource management issues.

#### Core Methods
| Method | Return Type | Description |
|--------|-------------|-------------|
| `bind(commandBuffer)` | `void` | Binds the pipeline to a command buffer |
| `defaultPipelineConfigInfo(configInfo)` | `static void` | Fills config with default values for standard 3D rendering |

### Private Members

#### Private Methods
| Method | Return Type | Description |
|--------|-------------|-------------|
| `readFile(filepath)` | `static std::vector<char>` | Reads binary file contents into a byte vector |
| `createGraphicsPipeline(vertCode, fragCode, configInfo)` | `void` | Creates the Vulkan graphics pipeline |
| `createShaderModule(code, shaderModule)` | `void` | Creates a shader module from SPIR-V code |

#### Private Data Members
| Member | Type | Description |
|--------|------|-------------|
| `dixdevice` | `EngineDevice&` | Reference to engine device |
| `graphicsPipeline` | `VkPipeline` | The graphics pipeline handle |
| `vertShaderModule` | `VkShaderModule` | Vertex shader module |
| `fragShaderModule` | `VkShaderModule` | Fragment shader module |

## Detailed Method Descriptions

### bind()
```cpp
void bind(VkCommandBuffer commandBuffer);
```
Binds the graphics pipeline to the provided command buffer.

**Operations:**
1. Calls `vkCmdBindPipeline` with:
   - Pipeline bind point: `VK_PIPELINE_BIND_POINT_GRAPHICS`
   - The graphics pipeline handle

**Usage:**
Must be called before drawing commands in a render pass.

### defaultPipelineConfigInfo()
```cpp
static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
```
Fills a `PipelineConfigInfo` structure with sensible defaults for 3D rendering.

**Default Settings:**
- **Viewport**: Single viewport covering entire framebuffer, dynamic state enabled
- **Scissor**: Single scissor rectangle covering entire framebuffer, dynamic state enabled
- **Input Assembly**: Triangle list topology, primitive restart disabled
- **Rasterization**:
  - Fill polygon mode
  - Backface culling
  - Clockwise front face
  - Depth bias disabled
- **Multisampling**: 1 sample (no MSAA)
- **Color Blending**:
  - No blending for attachment (source color overwrites)
  - Write mask for all channels (RGBA)
- **Depth Stencil**:
  - Depth test enabled
  - Depth write enabled
  - Less-or-equal comparison
- **Dynamic States**: Viewport and scissor rectangles

**Note:** After calling this function, you typically need to set:
- `pipelineLayout`: Create or assign a pipeline layout
- `renderPass`: Assign the render pass from swap chain
- `vertexBindingDescriptions`: Define your vertex format
- `vertexAttributeDescriptions`: Define vertex attributes

### readFile()
```cpp
static std::vector<char> readFile(const std::string& filepath);
```
Reads a binary file (SPIR-V shader) into memory.

**Operations:**
1. Opens file in binary mode
2. Seeks to end to get file size
3. Resizes vector to file size
4. Reads entire file into vector
5. Returns byte vector

**Error Handling:**
Throws runtime error if file cannot be opened.

### createGraphicsPipeline()
```cpp
void createGraphicsPipeline(
    const std::string& vertShaderCode,
    const std::string& fragShaderCode,
    const PipelineConfigInfo& pipelineInfo);
```
Creates the complete graphics pipeline.

**Operations:**
1. Creates shader modules for vertex and fragment shaders
2. Sets up pipeline shader stages (vertex and fragment)
3. Configures vertex input from `vertexBindingDescriptions` and `vertexAttributeDescriptions`
4. Builds pipeline create info structure with all state configurations
5. Calls `vkCreateGraphicsPipelines` to create the pipeline
6. Cleans up temporary shader modules after pipeline creation

### createShaderModule()
```cpp
void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
```
Creates a Vulkan shader module from SPIR-V bytecode.

**Operations:**
1. Fills `VkShaderModuleCreateInfo` with:
   - Code size
   - Pointer to SPIR-V code (cast to `uint32_t*`)
2. Calls `vkCreateShaderModule`

## Usage Example

### Basic Pipeline Creation
```cpp
// Create default config
dix::PipelineConfigInfo configInfo{};
dix::Pipeline::defaultPipelineConfigInfo(configInfo);

// Set required fields
configInfo.renderPass = swapChain.getRenderPass();
configInfo.pipelineLayout = pipelineLayout;

// Create pipeline
auto pipeline = std::make_unique<dix::Pipeline>(
    device,
    "shaders/simple_shader.vert.spv",
    "shaders/simple_shader.frag.spv",
    configInfo);
```

### Custom Vertex Input
```cpp
// Configure vertex input for custom vertex structure
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 uv;
};

std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
    Vertex::getBindingDescription()
};

std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
    Vertex::getPositionAttributeDescription(),
    Vertex::getColorAttributeDescription(),
    Vertex::getNormalAttributeDescription(),
    Vertex::getUvAttributeDescription()
};

configInfo.vertexBindingDescriptions = bindingDescriptions;
configInfo.vertexAttributeDescriptions = attributeDescriptions;

// Now create pipeline with custom vertex format
auto pipeline = std::make_unique<dix::Pipeline>(
    device,
    "shaders/vert.spv",
    "shaders/frag.spv",
    configInfo);
```

### Rendering with Pipeline
```cpp
// In render loop
VkCommandBuffer commandBuffer = ...;

vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

// Bind the pipeline
pipeline->bind(commandBuffer);

// Bind vertex buffers, descriptor sets, etc.
vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);

// Draw
vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);

vkCmdEndRenderPass(commandBuffer);
```

## Pipeline Lifecycle

1. **Creation**:
   - Read shader files from disk
   - Create shader modules
   - Configure pipeline states via `PipelineConfigInfo`
   - Call `vkCreateGraphicsPipelines`

2. **Usage**:
   - Bind pipeline in command buffer
   - Execute draw commands
   - Can be bound multiple times in different render passes

3. **Destruction**:
   - Destroy shader modules
   - Destroy pipeline
   - Must occur before destroying device and render pass

## Shader Module Details

The pipeline creates two shader modules:
- **Vertex Shader**: Processes vertices, transforms to clip space
- **Fragment Shader**: Determines pixel colors

Both shaders are loaded from SPIR-V binary files (compiled from GLSL/HLSL).

## Configuration Flexibility

The `PipelineConfigInfo` structure allows extensive customization:
- Different render passes
- Custom blend modes (transparency)
- Wireframe rendering (change polygon mode)
- Different culling modes
- Multisampling for anti-aliasing
- Stencil operations
- Custom dynamic states

## Notes
- Pipeline creation is expensive; create once and reuse
- Shader modules are destroyed after pipeline creation (not needed afterward)
- Pipeline is immutable once created; must recreate for different configurations
- Default config is suitable for most 3D opaque object rendering
- Pipeline must be bound before any draw commands
- Compatible with the render pass it was created for

## Dependencies
- **EngineDevice**: For device access and shader module creation
- **PipelineConfigInfo**: Configuration structure
- **Vulkan**: Graphics pipeline APIs
- **C++ Standard Library**: `<string>`, `<vector>`, `<cstdint>`, `<fstream>`

## Common Pipeline Configurations

### Opaque 3D Objects
Use `defaultPipelineConfigInfo()` with depth testing enabled.

### Transparent Objects
Modify `colorBlendAttachment` to enable alpha blending:
```cpp
configInfo.colorBlendAttachment.blendEnable = VK_TRUE;
configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
```

### Wireframe Rendering
Change rasterization polygon mode:
```cpp
configInfo.rasterizetionInfo.polygonMode = VK_POLYGON_MODE_LINE;
```

### No Culling
Disable backface culling:
```cpp
configInfo.rasterizetionInfo.cullMode = VK_CULL_MODE_NONE;
```