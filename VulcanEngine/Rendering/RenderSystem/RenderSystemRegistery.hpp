#ifndef RENDER_SYSTEM_REGISTERY_HPP
#define RENDER_SYSTEM_REGISTERY_HPP

// dix
#include <Rendering/RenderSystem/DixRenderSystem.hpp>
#include <Utils/DixConcepts.hpp>

namespace dix {
struct UboTypeInfo {
	size_t size;
	size_t alignment;	
};

struct RenderSystemConstructInfo {
	EngineDevice& engineDevice;
	VkRenderPass renderPass;
	VkDescriptorSetLayout globalSetLayout;
	VkDescriptorSetLayout modelSetLayout;
};

template <typename RenderSystem>
requires 
	HasUbos <RenderSystem> &&
	HasName <RenderSystem> 
	// HasVulkanFlags <RenderSystem>
	// is_tuple_v <typename RenderSystem::GlobalUbo>
struct RenderSystemDescription {
	RenderSystem* renderSystem;
	RenderSystem::Ubos Ubos;
	const char* renderSystemName = RenderSystem::Name();
};

template <typename... RenderSystems>
class RenderSystemRegistery {
	constexpr RenderSystemRegistery():
	m_renderSystems(std::make_tuple<RenderSystemDescription<RenderSystems>...>) {}

	std::tuple <RenderSystemDescription<RenderSystems>...>& getRenderSystemDescriptions() {
		return m_renderSystems;
	}
private:
	std::tuple <RenderSystemDescription<RenderSystems>...> m_renderSystems;
	//std::unordered_map<std::string, std::pair <std::unique_ptr<DixRenderSystem>, UboTypeInfo>> m_renderSystems;
};
}   // namepsace dix
#endif // RENDER_SYSTEM_REGISTERY_HPP