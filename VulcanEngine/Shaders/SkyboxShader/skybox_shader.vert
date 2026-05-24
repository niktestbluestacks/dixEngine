#version 460

layout(location = 0) in vec3 position;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec2 fragUV;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
} ubo;

void main() {
    mat4 view = mat4(mat3(ubo.view));

    vec4 pos =
        ubo.projection *
        view *
        vec4(position, 1.0);

    gl_Position = pos.xyww;

    fragUV = uv;
}