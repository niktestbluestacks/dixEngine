#version 450
layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;
layout(set=0, binding=0) uniform sampler2D fontSampler;

void main() {
    vec4 c = texture(fontSampler, fragUV);
    // premultiplied alpha handling not required, just output sampled color
    outColor = c;
}
