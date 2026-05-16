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
#include <Model/DixTexture/DixTexture.hpp>
#include <Utils/DixConcepts.hpp>
#include <UI/DixUIElement.hpp>
#include <Utils/Converter.hpp>
#include <Rendering/RenderSystem/RenderSystemRegistery.hpp>
#include <utility>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace dix {

template <typename ...RenderSystems>
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
		createDescriptorPool();
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
	VkRenderPass getSwapChainRenderPass() { return m_dixRenderer.getSwapChainRenderPass(); }
	VkCommandBuffer beginFrame() { return m_dixRenderer.beginFrame(); }
	void endFrame() { m_dixRenderer.endFrame(); }
	void beginSwapChainRenderPass(VkCommandBuffer cb) { m_dixRenderer.beginSwapChainRenderPass(cb); }
	void endSwapChainRenderPass(VkCommandBuffer cb) { m_dixRenderer.endSwapChainRenderPass(cb); }
	int getFrameIndex() { return m_dixRenderer.getFrameIndex(); }
	VkExtent2D getExtent() { return m_Window.getExtent(); }
	// device access for resource creation
	EngineDevice& device() { return m_dixDevice; }

	// draw helper which hides rendering/shader details from the app
	void drawFrame(
		DixCamera& camera, 
		float frameTime, 
		std::unordered_map<std::string, std::vector<GameObject>>& gameObjects, 
		const glm::vec3& playerPosition
	) {
		// if window is minimized or has zero area, skip rendering to avoid Vulkan errors
		auto extent = m_Window.getExtent();
		if (extent.width == 0 || extent.height == 0) return;

		// always update UI (do this before acquiring swapchain image) so UI logic
		// runs even when swapchain recreation causes beginFrame() to return null
		AdditionalUIInfo additionalInfo{
			.playerPosition = playerPosition
		};
		if (m_uiManager) {
			m_uiManager->update(frameTime, additionalInfo);
		}
		// TODO:
		// CHANGE THIS
		const auto& renderSystemName = std::get<0>(m_renderSystemRegistery.getRenderSystemDescriptions()).name;
		const auto& renderSystem = std::get<0>(m_renderSystemRegistery.getRenderSystemDescriptions())->renderSystem;
		const auto& uboSize = sizeof(std::get<0>(m_renderSystemRegistery.getRenderSystemDescriptions()).Ubos);
		// for (const auto &[renderSystemName, renderInfo] : DIX_RSR.getRenderSystems()) {
		if (auto commandBuffer = beginFrame()) {
			int frameIndex = getFrameIndex();
			FrameInfo frameInfo{
				frameIndex,
				frameTime,
				commandBuffer,
				camera,
				m_systemDescriptorSets[renderSystemName][frameIndex],
				m_Window.getExtent()
			};
			// allow UI elements to upload per-frame resources now that a frame and command buffer exist
			if (m_uiManager) {
				m_uiManager->upload(frameInfo);
			}

			// update global ubo for this frame
			decltype(std::get<0>(SimpleRenderSystem::Ubos())) ubo{};
			ubo.projectionView = camera.getProjection() * camera.getView();
			m_systemUboBuffers[renderSystemName][frameIndex]->writeToIndex(&ubo, 0);
			m_systemUboBuffers[renderSystemName][frameIndex]->flush();

			// UI was already updated before acquiring the swapchain image

			// render
			beginSwapChainRenderPass(commandBuffer);
			renderSystem.renderGameObjects(frameInfo, gameObjects[renderSystemName]);
			// render UI
			if (m_uiManager && m_uiRenderer) {
				m_uiRenderer->bindPipeline(commandBuffer);
				// push screen size to UI vertex shader (vec2)
				float screenSize[2] = { 
					static_cast<float>(m_Window.getExtent().width),
					static_cast<float>(m_Window.getExtent().height)
				};
				vkCmdPushConstants(
					commandBuffer,
					m_uiRenderer->getPipelineLayout(),
					VK_SHADER_STAGE_VERTEX_BIT,
					0,
					sizeof(screenSize),
					&screenSize);
				m_uiManager->render(frameInfo);
			}
			endSwapChainRenderPass(commandBuffer);
			endFrame();
		}
	}

	// 
	void addUIElement(std::unique_ptr<DixUIElement> element) {
		if (m_uiManager) {
        m_uiManager->addElement(std::move(element));
    	}
	}
	// void addGameObject(std::unique_ptr<GameObject> object);
	DixDescriptorPool& getDescriptorPool() { return *m_modelDescriptorPool; }
	DixDescriptorSetLayout& getModelSetLayout() { return *m_modelSetLayout; }

private:
	Window m_Window;
	EngineDevice m_dixDevice;
	Renderer m_dixRenderer;

	// rendering resources
	std::unique_ptr<DixDescriptorPool> m_globalPool;

	std::unordered_map<std::string, std::vector<std::unique_ptr<DixBuffer>>> m_systemUboBuffers;
	std::unordered_map<std::string, std::unique_ptr<DixDescriptorSetLayout>> m_systemSetLayouts;
	std::unordered_map<std::string, std::vector<VkDescriptorSet>> m_systemDescriptorSets;

 // per-model descriptor resources
	std::unique_ptr<DixDescriptorPool> m_modelDescriptorPool;
	std::unique_ptr<DixDescriptorSetLayout> m_modelSetLayout;

    std::unique_ptr<dix::UIManager> m_uiManager;
    std::unique_ptr<dix::UIRenderer> m_uiRenderer;
    // default fallback texture used for models when a mesh has no texture
	DixTexture m_defaultTexture;

	RenderSystemRegistery<RenderSystems...> m_renderSystemRegistery;

	template <typename RenderSystemInfo>
	void drawFrameForSingleRenderSystem(RenderSystemInfo&& info) {

	}

	void createUBOs() {
		std::apply([this](auto&& arg) {
			createSingeUbo(std::forward<decltype(arg)>(arg));
		}, m_renderSystemRegistery.getRenderSystemDescriptions());	
	}

	template <typename RenderSystemInfo>
	void createSingleUbo(RenderSystemInfo&& info) {
		m_systemUboBuffers[info.name].resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (auto& buf : m_systemUboBuffers[info.name]) {
			
			buf = std::make_unique<DixBuffer>(
				m_dixDevice,
				sizeof(info.Ubos),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			buf->map();
		}
	}
	void createDescriptorSets() {
		m_defaultTexture = createDefaultTexture(m_dixDevice);
		std::apply([this](auto&& arg) {
			createSingleDescriptorSet(std::forward<decltype(arg)>(arg));
		}, m_renderSystemRegistery.getRenderSystemDescriptions());
	}

	template <typename RenderSystemInfo>
	void createSingleDescriptorSet(RenderSystemInfo info) {
		m_systemDescriptorSets[info.name].resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_defaultTexture.getImageView();
		imageInfo.sampler = m_defaultTexture.getSampler();

		for (size_t i = 0; i < m_systemDescriptorSets[info.name].size(); ++i) {
			auto bufferInfo = m_systemUboBuffers[info.name][i]->descriptorInfo();
			DixDescriptorWriter(*m_systemSetLayouts[info.name], *m_globalPool)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1, &imageInfo)
				.build(m_systemDescriptorSets[info.name][i]);
		}
	}
	void createRenderSystems() {
		// TODO: CHANGE THIS
		std::get<0>(m_renderSystemRegistery.getRenderSystemDesctiptions())->renderSystem(
		m_dixDevice,
        m_dixRenderer.getSwapChainRenderPass(),
        m_systemSetLayouts["SimpleRenderSystem"]->getDescriptorSetLayout(),
        m_modelSetLayout->getDescriptorSetLayout());
	}

	
	void createModelDescriptorResources() {
		m_modelSetLayout = DixDescriptorSetLayout::Builder(m_dixDevice)
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

		// Create a large pool for model descriptor sets
		m_modelDescriptorPool = DixDescriptorPool::Builder(m_dixDevice)
			.setMaxSets(1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
			.build();
	}
	void createSystemSetLayouts() {
		// TODO: change it to iterato throe tuples of flags inside render system
		m_systemSetLayouts["SimpleRenderSystem"] = DixDescriptorSetLayout::Builder(m_dixDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();
	}
	void declareRenderSystems();
	void createDescriptorPool() {
		m_globalPool = DixDescriptorPool::Builder(m_dixDevice)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
	}
};

} // namespace dix

#endif // APP_CONTEXT_HPP
