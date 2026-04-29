#ifndef APP_CONTEXT_HPP
#define APP_CONTEXT_HPP

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>
#include <Pipeline/Buffer/DixBuffer.hpp>
#include <Rendering/Renderer/Renderer.hpp>
#include <Rendering/RenderSystem/SimpleRenderSystem/SimpleRenderSystem.hpp>
#include <Window/WindowClass/WindowClass.hpp>
#include <Utils/FrameInfo.hpp>
#include <DixCamera/DixCamera.hpp>
#include <Model/GameObject/GameObject.hpp>
#include <UI/UIManager.hpp>
#include <UI/UIRenderer.hpp>

// std
#include <memory>
#include <vector>

namespace dix {

class AppContext {
public:
	AppContext(int width, int height, const std::string& title);
	~AppContext();

	void initialize(); // New method to initialize resources
	// window/input
	bool shouldClose() { return m_Window.shouldClose(); }
	void pollEvents() { glfwPollEvents(); }
	GLFWwindow* getGLFWwindow() { return m_Window.getGLFWwindow(); }

	// renderer helpers
	float getAspectRatio() { return m_dixRenderer.getAspectRatio(); }
	VkRenderPass getSwapChainRenderPass() { return m_dixRenderer.getSwapChainRenderPass(); }
	VkCommandBuffer beginFrame() { return m_dixRenderer.beginFrame(); }
	void endFrame() { m_dixRenderer.endFrame(); }
	void beginSwapChainRenderPass(VkCommandBuffer cb) { m_dixRenderer.beginSwapChainRenderPass(cb); }
	void endSwapChainRenderPass(VkCommandBuffer cb) { m_dixRenderer.endSwapChainRenderPass(cb); }
	int getFrameIndex() { return m_dixRenderer.getFrameIndex(); }

	// device access for resource creation
	EngineDevice& device() { return m_dixDevice; }

	// draw helper which hides rendering/shader details from the app
	void drawFrame(DixCamera& camera, float frameTime, const std::vector<GameObject>& gameObjects);

private:
	Window m_Window;
	EngineDevice m_dixDevice;
	Renderer m_dixRenderer;

	// rendering resources
	std::unique_ptr<DixDescriptorPool> m_globalPool;
	std::unique_ptr<DixDescriptorSetLayout> m_globalSetLayout;
	std::vector<std::unique_ptr<DixBuffer>> m_uboBuffers;
	std::vector<VkDescriptorSet> m_globalDescriptorSets;

	std::unique_ptr<SimpleRenderSystem> m_simpleRenderSystem;
    std::unique_ptr<dix::UIManager> m_uiManager;
    std::unique_ptr<dix::UIRenderer> m_uiRenderer;
	void createDescriptorPool(); // New method to create descriptor pool
	void createUBOs(); // New method to create UBOs
	void createDescriptorSets(); // New method to create descriptor sets
	void createRenderSystem(); // New method to create simple render system
};

} // namespace dix

#endif // APP_CONTEXT_HPP
