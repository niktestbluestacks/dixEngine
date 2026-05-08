// dix
#include <Rendering/RenderSystem/SimpleRenderSystem/SimpleRenderSystem.hpp>

namespace dix {

struct SimplePushConstantData {
	glm::mat4 modelMatrix{ 1.f };
	glm::mat4 normalMatrix{ 1.f };
};

SimpleRenderSystem::SimpleRenderSystem(
		EngineDevice& engineDevice, 
		VkRenderPass renderPass, 
		VkDescriptorSetLayout globalSetLayout,
		VkDescriptorSetLayout modelSetLayout) :
		DixRenderSystem(
			engineDevice,
			renderPass,
			globalSetLayout,
			modelSetLayout,
			"SimpleShader/simple_shader.vert.spv",
			"SimpleShader/simple_shader.frag.spv",
			sizeof(SimplePushConstantData),
			[](void* pushConstantData, GameObject& obj) {
				auto* simplePush = static_cast<SimplePushConstantData*>(pushConstantData);
				simplePush->modelMatrix = obj.transform.mat4();
				simplePush->normalMatrix = obj.transform.normalMatrix();
			}
		) {}

}	// namespace dix