#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;

layout(binding = 0) uniform GlobalUbo {
    mat4 lightSpaceMatrix;
} ubo;

void main() {
    vec4 worldPosition = push.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = ubo.lightSpaceMatrix * worldPosition;
    fragColor = vec3(1.0, 1.0, 1.0);
}