#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragUV;

layout (set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
    mat4 view;
} ubo;

void main() {
	// Remove translation from view matrix for skybox (keep rotation only)
	mat4 viewWithoutTranslation = ubo.view;
	viewWithoutTranslation[3][0] = 0.0;
	viewWithoutTranslation[3][1] = 0.0;
	viewWithoutTranslation[3][2] = 0.0;

	gl_Position = ubo.projection * viewWithoutTranslation * vec4(position, 1.0);

	fragColor = color;
	fragUV = uv;
} 