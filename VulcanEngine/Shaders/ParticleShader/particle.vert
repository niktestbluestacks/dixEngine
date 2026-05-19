#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in float inLifetime;
layout(location = 2) in vec3 inVelocity;
layout(location = 3) in float inSize;
layout(location = 4) in vec4 inColor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out float fragSize;

layout(binding = 0) uniform ParticleUbo {
    mat4 projectionView;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;

void main() {
    vec4 worldPosition = push.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = ubo.projectionView * worldPosition;

    fragColor = inColor.rgb;
    fragSize = inSize;
}