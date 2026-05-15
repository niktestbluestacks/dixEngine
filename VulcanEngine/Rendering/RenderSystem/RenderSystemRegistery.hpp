#ifndef RENDER_SYSTEM_REGISTERY_HPP
#define RENDER_SYSTEM_REGISTERY_HPP

// dix
#include <Rendering/RenderSystem/DixRenderSystem.hpp>

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

class RenderSystemRegistery {
public:
	static RenderSystemRegistery& getInstance() {
		static RenderSystemRegistery instance;
		return instance;
	}

	DIX_DISABLE_COPY_AND_MOVE(RenderSystemRegistery)

	template<typename SystemType, typename UboStruct>
	void registerRenderSystem(const std::string& name, RenderSystemConstructInfo constructInfo) {
		assert(is_not_unique_instance(name) && "Render system with this name already exists in the registery!");
		assert(alignof(UboStruct) % 16 == 0 && "Uniform buffer struct alignment must be a multiple of 16 bytes!");
		UboTypeInfo info { sizeof(UboStruct), alignof(UboStruct) };
		m_renderSystems[name] = std::make_pair(std::make_unique<SystemType>(
			constructInfo.engineDevice,
			constructInfo.renderPass,
			constructInfo.globalSetLayout,
			constructInfo.modelSetLayout
		),
			info
		);
	}

	template <typename UboStruct>
	void declareRenderSystem(const std::string& name) {
		assert(is_not_unique_instance(name) && "Render system with this name already exists in the registery!");
		m_renderSystems[name] = std::make_pair(nullptr, UboTypeInfo{ sizeof(UboStruct), alignof(UboStruct) });
	}

	std::unique_ptr<DixRenderSystem>& getRenderSystem(const std::string& name) {
		assert(m_renderSystems.contains(name) && "No render system with this name exists in the registery!");
		return m_renderSystems[name].first;
	}

	UboTypeInfo getUboTypeInfo(const std::string& name) {
		assert(m_renderSystems.contains(name) && "No render system with this name exists in the registery!");
		return m_renderSystems[name].second;
	}

	const std::unordered_map<std::string, std::pair <std::unique_ptr<DixRenderSystem>, UboTypeInfo>>& getRenderSystems() const {
		return m_renderSystems;
	}
private:
	RenderSystemRegistery() = default;
	bool is_not_unique_instance(const std::string& name) {
		return (!m_renderSystems.contains(name) ? true : m_renderSystems[name].first == nullptr);
	}
	std::unordered_map<std::string, std::pair <std::unique_ptr<DixRenderSystem>, UboTypeInfo>> m_renderSystems;
};
}   // namepsace dix
#define DIX_RSR dix::RenderSystemRegistery::getInstance()
#endif // RENDER_SYSTEM_REGISTERY_HPP