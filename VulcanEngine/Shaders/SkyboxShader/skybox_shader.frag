#version 460

layout (location = 0) in vec2 fragUV;

layout (location = 0) out vec4 outColor;

layout(set = 1, binding = 1) uniform sampler2D texSampler;

void main() {
	outColor = texture(texSampler, fragUV);
	// for debugging:
	// outColor = vec4(1.0, 1.0, 0.0, 1.0);
} 