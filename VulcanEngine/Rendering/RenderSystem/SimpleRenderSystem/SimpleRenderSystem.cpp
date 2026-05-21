// dix
#include <Rendering/RenderSystem/SimpleRenderSystem/SimpleRenderSystem.hpp>

namespace dix {

SimpleRenderSystem::SimpleRenderSystem(
		EngineDevice& engineDevice, 
		VkRenderPass renderPass, 
		VkDescriptorSetLayout globalSetLayout,
		VkDescriptorSetLayout modelSetLayout,
		DixDescriptorPool& descriptorPool) :
		DixRenderSystem(
			engineDevice,
			renderPass,
			globalSetLayout,
			modelSetLayout,
			descriptorPool,
			"SimpleShader/simple_shader.vert.spv",
			"SimpleShader/simple_shader.frag.spv",
			[](void* pushConstantData, GameObject& obj) {
				auto* simplePush = static_cast<SimplePushConstantData*>(pushConstantData);
				simplePush->modelMatrix = obj.transform.mat4();
				simplePush->normalMatrix = obj.transform.normalMatrix();
			}
		) {
			createPipelineLayout(globalSetLayout, modelSetLayout);
			createPipeline(renderPass);
		}

}	// namespace dix