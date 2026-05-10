#ifndef DIX_RENDER_SYSTEM_HPP
#define DIX_RENDER_SYSTEM_HPP

// dix
#include <DixCamera/DixCamera.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Model/GameObject/GameObject.hpp>
#include <Pipeline/Pipeline/Pipeline.hpp>
#include <Utils/FrameInfo.hpp>
#include <Utils/Class.hpp>

// std
#include <memory>
#include <vector>
#include <concepts>

namespace dix {

class DixRenderSystem {
protected:
    virtual void createPipelineLayout(
        VkDescriptorSetLayout globalSetLayout, 
        VkDescriptorSetLayout modelSetLayout
    );
	virtual void createPipeline(VkRenderPass renderPass);
public:

	DixRenderSystem(
		EngineDevice& engineDeivce, 
		VkRenderPass renderPass, 
		VkDescriptorSetLayout globalSetLayout,
		VkDescriptorSetLayout modelSetLayout,
        std::string vertShaderBinaryPath, 
        std::string fragShaderBinaryPath,
        int sizeofPushConstantData,
        std::function<void(void*, GameObject&)> transformGameObject
	);
	virtual ~DixRenderSystem(void);

	DIX_DISABLE_COPY(DixRenderSystem)

	virtual void renderGameObjects(
		FrameInfo& frameInfo, 
		std::vector <GameObject>& gameObjects
	);
protected:
    std::string m_vertShaderBinaryPath;
    std::string m_fragShaderBinaryPath;

	EngineDevice& m_dixDevice;
    std::unique_ptr<Pipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
    std::function<void(void*, GameObject&)> m_transformGameObject;
    int m_sizeofPushConstantData;
};

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
	bool is_not_unique_instance(const std::string& name);
	std::unordered_map<std::string, std::pair <std::unique_ptr<DixRenderSystem>, UboTypeInfo>> m_renderSystems;
};

}	// namespace dix

#define DIX_RSR dix::RenderSystemRegistery::getInstance()
#endif // DIX_RENDER_SYSTEM_HPP