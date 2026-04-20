#version 450
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 0) out vec2 fragUV;

void main() {
    // expecting positions in pixel space; convert to NDC in shader
    // Viewport size will be supplied via push constants or UBO in future
    vec2 ndc = inPos; // caller should provide already normalized coords
    gl_Position = vec4(ndc, 0.0, 1.0);
    fragUV = inUV;
}
