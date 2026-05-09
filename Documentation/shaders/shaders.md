--- documentation/shaders/shaders.md (原始)


+++ documentation/shaders/shaders.md (修改后)
# Shader Documentation

## Overview
This document covers all shader programs used in the VulcanEngine. Shaders are written in GLSL and compiled to SPIR-V bytecode for Vulkan consumption.

## Shader Files

### 1. Simple Shader (3D Rendering)
**Location**: `VulcanEngine/Shaders/SimpleShader/`

#### Vertex Shader: `simple_shader.vert`
**Purpose**: Transforms 3D vertices to clip space and calculates lighting.

**Version**: GLSL 4.60

**Inputs (Vertex Attributes)**:
| Location | Type | Name | Description |
|----------|------|------|-------------|
| 0 | `vec3` | `position` | Vertex position in object space |
| 1 | `vec3` | `color` | Vertex color |
| 2 | `vec3` | `normal` | Surface normal for lighting |
| 3 | `vec2` | `uv` | Texture coordinates |

**Outputs (to Fragment Shader)**:
| Location | Type | Name | Description |
|----------|------|------|-------------|
| 0 | `vec3` | `fragColor` | Lit vertex color |
| 1 | `vec2` | `fragUV` | Texture coordinates |

**Uniform Buffers**:
```glsl
// Set 0, Binding 0
uniform GlobalUbo {
    mat4 projectionViewMatrix;  // Combined projection and view matrix
    vec3 directionToLight;      // Direction to light source (normalized)
} ubo;
```

**Push Constants**:
```glsl
layout(push_constant) uniform Push {
    mat4 modelMatrix;   // Object-to-world transformation
    mat4 normalMatrix;  // Normal transformation matrix (inverse transpose of model)
} push;
```

**Constants**:
```glsl
const float AMBIENT = 0.02;  // Ambient lighting factor (2%)
```

**Operations**:
1. Transforms vertex position to clip space:
   ```glsl
   gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(position, 1.0);
   ```

2. Calculates world-space normal:
   ```glsl
   vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);
   ```

3. Computes Lambertian lighting with ambient term:
   ```glsl
   float lightIntensity = AMBIENT + max(dot(normalWorldSpace, ubo.directionToLight), 0);
   fragColor = lightIntensity * color;
   ```

4. Passes UV coordinates to fragment shader:
   ```glsl
   fragUV = uv;
   ```

**Lighting Model**:
- **Ambient**: Constant 2% minimum brightness
- **Diffuse**: Lambertian dot product with light direction
- **No Specular**: Simple diffuse-only lighting

---

#### Fragment Shader: `simple_shader.frag`
**Purpose**: Samples texture and combines with interpolated vertex color.

**Version**: GLSL 4.60

**Inputs (from Vertex Shader)**:
| Location | Type | Name | Description |
|----------|------|------|-------------|
| 0 | `vec3` | `fragColor` | Interpolated lit color from vertex shader |
| 1 | `vec2` | `fragUV` | Interpolated texture coordinates |

**Output**:
| Location | Type | Name | Description |
|----------|------|------|-------------|
| 0 | `vec4` | `outColor` | Final pixel color with alpha |

**Sampler Uniforms**:
```glsl
// Set 1, Binding 1
uniform sampler2D texSampler;  // Diffuse texture sampler
```

**Push Constants** (unused in fragment shader):
```glsl
layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;
```

**Operations**:
1. Samples texture at interpolated UV coordinates:
   ```glsl
   vec4 texColor = texture(texSampler, fragUV);
   ```

2. Multiplies texture color by vertex color (which includes lighting):
   ```glsl
   outColor = texColor * vec4(fragColor, 1.0);
   ```

**Color Calculation**:
- **Texture Modulation**: Texture RGB multiplied by vertex color RGB
- **Alpha**: Always set to 1.0 (opaque)
- **Lighting**: Already baked into `fragColor` from vertex shader

---

### 2. UI Shader (User Interface)
**Location**: `VulcanEngine/Shaders/UI/`

#### Vertex Shader: `ui.vert`
**Purpose**: Transforms 2D UI elements from pixel coordinates to normalized device coordinates.

**Version**: GLSL 4.50

**Inputs (Vertex Attributes)**:
| Location | Type | Name | Description |
|----------|------|------|-------------|
| 0 | `vec2` | `inPos` | Position in pixel coordinates |
| 1 | `vec2` | `inUV` | Texture/UV coordinates |

**Outputs (to Fragment Shader)**:
| Location | Type | Name | Description |
|----------|------|------|-------------|
| 0 | `vec2` | `fragUV` | UV coordinates for texturing |

**Push Constants**:
```glsl
layout(push_constant) uniform Push {
    vec2 screenSize;  // Screen dimensions in pixels (width, height)
} push;
```

**Operations**:
1. Converts pixel X coordinate to NDC [-1, 1]:
   ```glsl
   ndc.x = (inPos.x / push.screenSize.x) * 2.0 - 1.0;
   ```

2. Converts pixel Y coordinate to NDC [-1, 1] with Y-axis inversion:
   ```glsl
   ndc.y = 1.0 - (inPos.y / push.screenSize.y) * 2.0;
   ```
   - Inverts Y because UI coordinates typically have Y-down convention
   - Vulkan NDC has Y-up convention

3. Sets depth to 0.0 (UI rendered at near plane) and homogeneous coordinate to 1.0:
   ```glsl
   gl_Position = vec4(ndc, 0.0, 1.0);
   ```

4. Passes UV coordinates to fragment shader:
   ```glsl
   fragUV = inUV;
   ```

**Coordinate System**:
- **Input**: Pixel coordinates (origin at top-left)
- **Output**: NDC coordinates (origin at center, Y-up)
- **Example**: For 1920x1080 screen:
  - Pixel (0, 0) → NDC (-1, 1) - top-left
  - Pixel (1920, 1080) → NDC (1, -1) - bottom-right
  - Pixel (960, 540) → NDC (0, 0) - center

---

#### Fragment Shader: `ui.frag`
**Purpose**: Samples UI texture (typically font atlas or UI element texture).

**Version**: GLSL 4.50

**Inputs (from Vertex Shader)**:
| Location | Type | Name | Description |
|----------|------|------|-------------|
| 0 | `vec2` | `fragUV` | Interpolated texture coordinates |

**Output**:
| Location | Type | Name | Description |
|----------|------|------|-------------|
| 0 | `vec4` | `outColor` | Final pixel color with alpha |

**Sampler Uniforms**:
```glsl
// Set 0, Binding 0
uniform sampler2D fontSampler;  // Font atlas or UI texture sampler
```

**Operations**:
1. Samples texture at UV coordinates:
   ```glsl
   vec4 c = texture(fontSampler, fragUV);
   ```

2. Outputs sampled color directly:
   ```glsl
   outColor = c;
   ```

**Notes**:
- No color modulation or blending calculations in shader
- Alpha channel preserved from texture
- Premultiplied alpha handling done on CPU side if needed
- Suitable for:
  - Font rendering (signed distance fields or bitmap fonts)
  - UI element textures
  - Image display in UI

---

## Descriptor Set Layouts

### Simple Shader
| Set | Binding | Type | Name | Stage |
|-----|---------|------|------|-------|
| 0 | 0 | Uniform Buffer | `GlobalUbo` | Vertex |
| 1 | 1 | Sampler | `texSampler` | Fragment |

### UI Shader
| Set | Binding | Type | Name | Stage |
|-----|---------|------|------|-------|
| 0 | 0 | Sampler | `fontSampler` | Fragment |

---

## Push Constant Ranges

### Simple Shader
| Stage | Offset | Size | Contents |
|-------|--------|------|----------|
| Vertex | 0 | 64 bytes | `modelMatrix` (mat4) + `normalMatrix` (mat4) |
| Fragment | 0 | 64 bytes | Same as vertex (available but unused) |

### UI Shader
| Stage | Offset | Size | Contents |
|-------|--------|------|----------|
| Vertex | 0 | 8 bytes | `screenSize` (vec2) |

---

## Compilation

Shaders must be compiled from GLSL to SPIR-V using `glslc`:

```bash
# Compile simple shader
glslc simple_shader.vert -o simple_shader.vert.spv
glslc simple_shader.frag -o simple_shader.frag.spv

# Compile UI shader
glslc ui.vert -o ui.vert.spv
glslc ui.frag -o ui.frag.spv
```

Or use the provided batch script:
```batch
Shaders\compile.bat
```

---

## Usage Examples

### Simple Shader (3D Objects)
```cpp
// Set up global UBO (per-frame)
struct GlobalUbo {
    glm::mat4 projectionViewMatrix;
    glm::vec3 directionToLight;
};

// Update UBO
GlobalUbo ubo{};
ubo.projectionViewMatrix = camera.getProjection() * camera.getView();
ubo.directionToLight = glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f));

// Set push constants (per-object)
struct PushConstants {
    glm::mat4 modelMatrix;
    glm::mat4 normalMatrix;
};

PushConstants push{};
push.modelMatrix = gameObject.transform.getMatrix();
push.normalMatrix = glm::transpose(glm::inverse(push.modelMatrix));

vkCmdPushConstants(commandBuffer, pipelineLayout,
                   VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push), &push);
```

### UI Shader (2D Interface)
```cpp
// Set push constants (per-frame or per-resolution-change)
struct UiPushConstants {
    glm::vec2 screenSize;
};

UiPushConstants push{};
push.screenSize = glm::vec2(windowWidth, windowHeight);

vkCmdPushConstants(commandBuffer, pipelineLayout,
                   VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push), &push);

// Draw UI quads with pixel coordinates
// Vertex data: [pixelX, pixelY, uvX, uvY]
```

---

## Performance Considerations

### Simple Shader
- **Vertex Processing**: Moderate (matrix multiplications + lighting)
- **Fragment Processing**: Light (single texture sample + multiply)
- **Memory**: Uses 2 descriptor sets (global UBO + texture)
- **Optimizations**:
  - Lighting calculated in vertex shader (Gouraud shading)
  - Could move to fragment shader for Phong shading (more accurate, more expensive)

### UI Shader
- **Vertex Processing**: Minimal (simple coordinate conversion)
- **Fragment Processing**: Minimal (single texture sample)
- **Memory**: Single descriptor set (font atlas)
- **Optimizations**:
  - Very efficient for batched UI rendering
  - Supports signed distance field fonts for crisp text at any scale

---

## Common Modifications

### Simple Shader Enhancements
1. **Add Specular Lighting**:
   ```glsl
   vec3 viewDir = normalize(cameraPos - worldPos);
   vec3 reflectDir = reflect(-lightDir, normalWorldSpace);
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
   fragColor += spec * specularStrength;
   ```

2. **Add Fog**:
   ```glsl
   float fogFactor = exp(-fogDensity * distance);
   fragColor = mix(fogColor, fragColor, fogFactor);
   ```

3. **Add Point Lights**:
   Replace directional light with position-based lighting and attenuation.

### UI Shader Enhancements
1. **Add Color Tint**:
   Add vertex color attribute and multiply with sampled texture.

2. **Add Scissor Test**:
   Implement in shader or use Vulkan scissor rectangles.

3. **Add SDF Font Support**:
   Modify fragment shader for signed distance field font rendering:
   ```glsl
   float distance = texture(fontSampler, fragUV).a;
   float alpha = smoothstep(0.5 - pixelRange, 0.5 + pixelRange, distance);
   outColor = vec4(color.rgb, color.a * alpha);
   ```

---

## Dependencies

### Simple Shader Requires
- Vertex buffer with position, color, normal, UV attributes
- Global UBO with projection-view matrix and light direction
- Diffuse texture bound to set 1, binding 1
- Push constants with model and normal matrices

### UI Shader Requires
- Vertex buffer with 2D pixel positions and UVs
- Font/UI texture bound to set 0, binding 0
- Push constants with screen dimensions
- Orthographic projection (handled in shader)

---

## Debugging Tips

1. **White/Black Output**: Check texture binding and UV coordinates
2. **Incorrect Lighting**: Verify normal matrix calculation (inverse transpose)
3. **UI Misaligned**: Check screen size in push constants and Y-inversion
4. **Missing Geometry**: Verify vertex attribute locations match pipeline config
5. **Texture Artifacts**: Check sampler settings (filtering, wrap mode)