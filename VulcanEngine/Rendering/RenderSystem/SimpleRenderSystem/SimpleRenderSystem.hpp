#ifndef SIMPLE_RENDER_SYSTEM_HPP
#define SIMPLE_RENDER_SYSTEM_HPP

// dix
#include <Rendering/RenderSystem/DixRenderSystem.hpp>

namespace dix {
class SimpleRenderSystem : public DixRenderSystem {
public:
	using DixRenderSystem::DixRenderSystem;	// inherit constructors

	SimpleRenderSystem(
		EngineDevice& engineDeivce, 
		VkRenderPass renderPass, 
		VkDescriptorSetLayout globalSetLayout,
		VkDescriptorSetLayout modelSetLayout
	);

	DIX_DISABLE_COPY(SimpleRenderSystem)

};	// class SimpleRenderSystem
}	// namespace dix

#endif // SIMPLE_RENDER_SYSTEM_HPP