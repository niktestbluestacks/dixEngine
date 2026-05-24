#version 460

layout(location = 0) in vec4 inPosLife;
layout(location = 1) in vec4 inVelocitySize;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out float fragSize;

layout(binding = 0) uniform ParticleUbo {
    mat4 projectionView;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;

void main() {
    vec4 worldPosition = push.modelMatrix * vec4(inPosLife.xyz, 1.0);
    gl_Position = ubo.projectionView * worldPosition;

    fragColor = inColor.rgb;
    fragSize = inVelocitySize.w;
    gl_PointSize = fragSize;
}