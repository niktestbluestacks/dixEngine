#ifndef APP_CONTEXT_HPP
#define APP_CONTEXT_HPP

// dix
#include <FirstApp/AppInclude.hpp>

// std
#include <utility>

namespace dix {

template <typename... RenderSystems>
class AppContext {
public:
	AppContext(int width, int height, const std::string& title):
	m_Window{ width, height, title },
    m_dixDevice{ m_Window },
    m_dixRenderer{ m_Window, m_dixDevice } {

    	initialize();
    	m_uiManager = std::make_unique<dix::UIManager>();
    	m_uiRenderer = std::make_unique<dix::UIRenderer>(m_dixDevice, m_dixRenderer.getSwapChainRenderPass());
	}
	~AppContext() = default;

	void initialize() {
		createDescriptorPools();
		createUBOs();
		createSystemSetLayouts();
		createDescriptorSets();
		createModelDescriptorResources();
		createRenderSystems();
	}
	// window/input
	bool shouldClose() { return m_Window.shouldClose(); }
	void pollEvents() { glfwPollEvents(); }
	GLFWwindow* getGLFWwindow() { return m_Window.getGLFWwindow(); }
	std::unique_ptr<UIRenderer>& getUIRenderer() { return m_uiRenderer; }
	// renderer helpers
	float getAspectRatio() { return m_dixRenderer.getAspectRatio(); }
	vk::RenderPass getSwapChainRenderPass() { return m_dixRenderer.getSwapChainRenderPass(); }
	vk::CommandBuffer beginFrame() { return m_dixRenderer.beginFrame(); }
	void endFrame() { m_dixRenderer.endFrame(); }
	void beginSwapChainRenderPass(vk::CommandBuffer cb) { m_dixRenderer.beginSwapChainRenderPass(cb); }
	void endSwapChainRenderPass(vk::CommandBuffer cb) { m_dixRenderer.endSwapChainRenderPass(cb); }
	int getFrameIndex() { return m_dixRenderer.getFrameIndex(); }
	vk::Extent2D getExtent() { return m_Window.getExtent(); }
	// device access for resource creation
	EngineDevice& device() { return m_dixDevice; }

	// draw helper which hides rendering/shader details from the app
	void drawFrame(
		DixCamera& camera, 
		float frameTime, 
		std::unordered_map<std::string, std::vector<GameObject>>& gameObjects, 
		const glm::vec3& playerPosition
	);

	// 
	void addUIElement(std::unique_ptr<DixUIElement> element) {
		if (m_uiManager) {
        	m_uiManager->addElement(std::move(element));
    	}
	}
	// void addGameObject(std::unique_ptr<GameObject> object);
	DixDescriptorPool& getDescriptorPool() { return *m_modelDescriptorPool; }
	DixDescriptorSetLayout& getModelSetLayout() { return *m_modelSetLayout; }

	void shutdown() {
		m_uiManager.reset();
		m_uiRenderer.reset();
	}

	template <typename RenderSystem>
	decltype(auto) getRenderSystem() {
		return m_renderSystemRegistery.template getRenderSystem<RenderSystem>();
	}

	Window& getDixWindow() { return m_Window; }
private:
	Window m_Window;
	EngineDevice m_dixDevice;
	Renderer m_dixRenderer;

	// rendering resources
	std::unordered_map<std::string, std::unique_ptr<DixDescriptorPool>> m_systemPool;

	std::unordered_map<std::string, std::vector<std::vector<std::unique_ptr<DixBuffer>>>> m_systemUboBuffers;
	std::unordered_map<std::string, std::unique_ptr<DixDescriptorSetLayout>> m_systemSetLayouts;
        std::unordered_map<std::string, std::vector<vk::DescriptorSet>> m_systemDescriptorSets;

 	// per-model descriptor resources
	std::unique_ptr<DixDescriptorPool> m_modelDescriptorPool;
	std::unique_ptr<DixDescriptorSetLayout> m_modelSetLayout;

    std::unique_ptr<dix::UIManager> m_uiManager;
    std::unique_ptr<dix::UIRenderer> m_uiRenderer;
    // default fallback texture used for models when a mesh has no texture
	DixTexture m_defaultTexture;

	RenderSystemRegistery<RenderSystems...> m_renderSystemRegistery;

	void createUBOs();

	template <typename RenderSystemInfo>
	void createSingleUbo(RenderSystemInfo&& info);

	void createDescriptorSets();

	template <typename RenderSystemInfo>
	void createSingleDescriptorSet(RenderSystemInfo&& info);
	void createRenderSystems();

	template<size_t... Indices>
	void createRenderSystemsImpl(std::index_sequence<Indices...>);
	
	void createModelDescriptorResources();
	void createSystemSetLayouts();

	void createDescriptorPools();
	template <typename RenderSystemInfo>
	void createSingleDescriptorPool(RenderSystemInfo&& info);
};

} // namespace dix

#include <FirstApp/AppContext.tpp>

#endif // APP_CONTEXT_HPP
