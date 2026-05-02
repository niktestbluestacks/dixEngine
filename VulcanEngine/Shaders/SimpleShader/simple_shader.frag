#version 460

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragUV; // new 

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D texSampler;	// new

layout (push_constant) uniform Push {
	mat4 modelMatrix;	
	mat4 normalMatrix;
} push;

void main() {
	vec4 texColor = texture(texSampler, fragUV); // new
	if (texColor.a == 0.0) texColor = vec4(1.0);
	outColor = texColor * vec4(fragColor, 1.0);
} 