#version 460

layout(location = 0) in vec2 inPos; // pixel coordinates
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;
layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 fragColor;

layout(push_constant) uniform Push {
    vec2 screenSize;
} push;

void main() {
    // convert pixel coords to NDC [-1,1]
    vec2 ndc;
    ndc.x = (inPos.x / push.screenSize.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (inPos.y / push.screenSize.y) * 2.0; // invert Y
    gl_Position = vec4(ndc, 0.0, 1.0);
    fragUV = inUV;
    fragColor = inColor;
}
