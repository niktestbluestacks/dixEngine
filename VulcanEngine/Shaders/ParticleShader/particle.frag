#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in float fragSize;

layout(location = 0) out vec4 outColor;

void main() {
    // Simple point sprite - could be extended with texture lookup
    // Discard based on distance from center for circular particles

    vec2 center = gl_PointCoord - vec2(0.5);
    float distSq = dot(center, center);

    if (distSq > 0.5) {
        discard;
    }

    outColor = vec4(fragColor, 1.0);
}