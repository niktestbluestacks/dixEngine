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

layout (push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main() {
	gl_Position = ubo.projection * ubo.view * push.modelMatrix * vec4(position, 1.0);
	
	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

	fragColor = color;
	fragUV = uv; // new
} 