#ifndef APP_CONTEXT_HPP
#define APP_CONTEXT_HPP

// dix
#include <FirstApp/AppInclude.hpp>

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

		if (auto commandBuffer = beginFrame()) {
			int frameIndex = getFrameIndex();

			// Update UBOs for all systems first
			std::apply([&](auto&&... renderSystemDescs) {
				(([&](auto&& desc) {
					const auto& renderSystemName = desc.renderSystemName;

					// Update UBO for this system
					int uboTypeIndex = 0;
					std::apply([&](auto&&... uboArgs) {
						(([&](auto&& arg) {
							using UboType = std::remove_reference_t<decltype(arg)>;
							UboType ubo{};
							ubo.projectionView = camera.getProjection() * camera.getView();
							m_systemUboBuffers[renderSystemName][frameIndex][uboTypeIndex]->writeToBuffer(&ubo, sizeof(UboType));
							m_systemUboBuffers[renderSystemName][frameIndex][uboTypeIndex]->flush();

							// Update descriptor set with new buffer info
							VkDescriptorBufferInfo bufferInfo = m_systemUboBuffers[renderSystemName][frameIndex][uboTypeIndex]->descriptorInfo();
							DixDescriptorWriter writer(*m_systemSetLayouts[renderSystemName], *m_systemDescriptorPools[renderSystemName]);
							writer.writeBuffer(0, &bufferInfo);
							writer.overwrite(m_systemDescriptorSets[renderSystemName][frameIndex]);

							++uboTypeIndex;
						}(std::get<0>(std::tuple<std::decay_t<decltype(uboArgs)>>{}))), ...);
					}, desc.Ubos);
				}(renderSystemDescs)), ...);
			}, m_renderSystemRegistery.getRenderSystemDescriptions());

			std::string uiSystemName = std::get<0>(m_renderSystemRegistery.getRenderSystemDescriptions()).renderSystemName;
    
			FrameInfo uiFrameInfo{
				frameIndex,
				frameTime,
				commandBuffer,
				camera,
				m_systemDescriptorSets[uiSystemName][frameIndex], // Ensure this key exists!
				m_Window.getExtent()
			};
			// // allow UI elements to upload per-frame resources now that a frame and command buffer exist
			if (m_uiManager) {
				m_uiManager->upload(uiFrameInfo);
			}

			// render
			beginSwapChainRenderPass(commandBuffer);

			std::apply([&](auto&&... renderSystemDescs) {
				(([&](auto&& desc) {
					const auto& renderSystemName = desc.renderSystemName;

					FrameInfo frameInfo{
						frameIndex,
						frameTime,
						commandBuffer,
						camera,
						m_systemDescriptorSets[renderSystemName][frameIndex],
						m_Window.getExtent()
					};

					// Update UBO for this system
					int IndexOfWriteToIndex = 0;
					int uboTypeIndex = 0;
					std::apply([&](auto&&... uboArgs) {
						(([&](auto&& arg) {
							using UboType = std::remove_reference_t<decltype(arg)>;
							UboType ubo{};
							ubo.projectionView = camera.getProjection() * camera.getView();
							// Access: [renderSystemName][frameIndex][uboTypeIndex]
							m_systemUboBuffers[renderSystemName][frameIndex][uboTypeIndex]->writeToBuffer(&ubo, sizeof(UboType));
							m_systemUboBuffers[renderSystemName][frameIndex][uboTypeIndex]->flush();
							++uboTypeIndex;
							}(std::get<0>(std::tuple<std::decay_t<decltype(uboArgs)>>{}))), ...);
					}, desc.Ubos);

					// Render geometry
					desc.renderSystem->renderGameObjects(frameInfo, gameObjects[renderSystemName]);

				}(renderSystemDescs)), ...);
			}, m_renderSystemRegistery.getRenderSystemDescriptions());

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
					&screenSize
				);

				 // Use first system's descriptor set for UI
				std::string uiSystemName = std::get<0>(m_renderSystemRegistery.getRenderSystemDescriptions()).renderSystemName;
				FrameInfo uiFrameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					m_systemDescriptorSets[uiSystemName][frameIndex],
					m_Window.getExtent()
				};

				m_uiManager->render(uiFrameInfo);
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
	DixDescriptorPool& getSystemDescriptorPool(const std::string& systemName) {
		auto it = m_systemDescriptorPools.find(systemName);
		if (it == m_systemDescriptorPools.end()) {
			throw std::runtime_error("Descriptor pool not found for system: " + systemName);
		}
		return *it->second;
	}
	DixDescriptorSetLayout& getModelSetLayout() { return *m_modelSetLayout; }

	void shutdown() {
		m_uiManager.reset();
		m_uiRenderer.reset();
	}

private:
	Window m_Window;
	EngineDevice m_dixDevice;
	Renderer m_dixRenderer;

	// per-render-system descriptor pools and sets
	std::unordered_map<std::string, std::unique_ptr<DixDescriptorPool>> m_systemDescriptorPools;

	std::unordered_map<std::string, std::vector<std::vector<std::unique_ptr<DixBuffer>>>> m_systemUboBuffers;
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

	void createUBOs() {
		std::apply([this](auto&&... arg) {
			(createSingleUbo(std::forward<decltype(arg)>(arg)), ...);
		}, m_renderSystemRegistery.getRenderSystemDescriptions());	
	}

	template <typename RenderSystemInfo>
	void createSingleUbo(RenderSystemInfo&& info) {
		constexpr size_t uboCount = std::tuple_size_v<std::remove_reference_t<decltype(info.Ubos)>>;

		// Resize outer vector: [frameIndex][uboTypeIndex]
		m_systemUboBuffers[info.renderSystemName].resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		 // For each frame, create buffers for each UBO type
		for (auto& frameBuffers : m_systemUboBuffers[info.renderSystemName]) {
			frameBuffers.resize(uboCount);

			size_t uboTypeIndex = 0;
			std::apply([&](auto&&... uboTypes) {
				(([&](auto&& uboTypeInstance) {
					using UboType = std::decay_t<decltype(uboTypeInstance)>;
					VkDeviceSize bufferSize = sizeof(UboType);

					frameBuffers[uboTypeIndex] = std::make_unique<DixBuffer>(
					m_dixDevice,
					bufferSize,
					1,
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
					);
					frameBuffers[uboTypeIndex]->map();
					++uboTypeIndex;
				}(std::get<0>(std::tuple<std::decay_t<decltype(uboTypes)>>{}))), ...);
			}, info.Ubos);
		}
	}
	void createDescriptorSets() {
		m_defaultTexture = createDefaultTexture(m_dixDevice);
		std::apply([this](auto&&... arg) {
			(createSingleDescriptorSet(std::forward<decltype(arg)>(arg)), ...);
		}, m_renderSystemRegistery.getRenderSystemDescriptions());
	}

	template <typename RenderSystemInfo>
	void createSingleDescriptorSet(RenderSystemInfo&& info) {
		m_systemDescriptorSets[info.renderSystemName].resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_defaultTexture.getImageView();
		imageInfo.sampler = m_defaultTexture.getSampler();

		for (size_t i = 0; i < m_systemDescriptorSets[info.renderSystemName].size(); ++i) {
			// Create empty descriptor sets - they will be updated per-frame with overwrite()
			DixDescriptorWriter writer(*m_systemSetLayouts[info.renderSystemName], *m_systemDescriptorPools[info.renderSystemName]);
			writer.build(m_systemDescriptorSets[info.renderSystemName][i]);
		}
	}
	void createRenderSystems() {
		createRenderSystemsImpl(std::index_sequence_for<RenderSystems...>{});
	}

	template<size_t... Indices>
	void createRenderSystemsImpl(std::index_sequence<Indices...>) {
		(([&]() {
			using T = std::remove_reference_t<decltype(*std::get<Indices>(m_renderSystemRegistery.getRenderSystemDescriptions()).renderSystem)>;
			std::get<Indices>(m_renderSystemRegistery.getRenderSystemDescriptions()).renderSystem = std::make_unique<T>(
				m_dixDevice,
				m_dixRenderer.getSwapChainRenderPass(),
				m_systemSetLayouts[std::get<Indices>(m_renderSystemRegistery.getRenderSystemDescriptions()).renderSystemName]->getDescriptorSetLayout(),
				m_modelSetLayout->getDescriptorSetLayout(),
				*m_systemDescriptorPools[std::get<Indices>(m_renderSystemRegistery.getRenderSystemDescriptions()).renderSystemName]
			);
		})(), ...);
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
		auto processRenderSystem = [&](auto&& renderSystemDesc) {
        	auto&& vulkanFlags = renderSystemDesc.renderSystem->getVulkanFlags();
        	auto builder = DixDescriptorSetLayout::Builder(m_dixDevice);
			std::apply([&](auto&&... bindingTuples) {
				(std::apply([&](auto&&... args) {
					builder.addBinding(std::forward<decltype(args)>(args)...);
				}, bindingTuples), ...); 
			}, vulkanFlags);
        	m_systemSetLayouts[renderSystemDesc.renderSystemName] = builder.build();
    	};
    
    	std::apply([&](auto&&... args) {
        	(processRenderSystem(args), ...);
    	}, m_renderSystemRegistery.getRenderSystemDescriptions());
	}
	void createDescriptorPool() {
		std::apply([this](auto&&... renderSystemDesc) {
			(createSingleDescriptorPool(renderSystemDesc), ...);
		}, m_renderSystemRegistery.getRenderSystemDescriptions());
	}

	template <typename RenderSystemInfo>
	void createSingleDescriptorPool(RenderSystemInfo&& info) {
		constexpr size_t uboCount = std::tuple_size_v<std::remove_reference_t<decltype(info.Ubos)>>;
		auto vulkanFlags = info.renderSystem->getVulkanFlags();

		// Count descriptor types needed
		size_t uniformBufferCount = 0;
		size_t storageBufferCount = 0;
		size_t combinedImageSamplerCount = 0;

		std::apply([&](auto&&... bindingTuples) {
			(std::apply([&](auto binding, auto type, auto stages) {
				if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
					uniformBufferCount += SwapChain::MAX_FRAMES_IN_FLIGHT;
				} else if (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
					storageBufferCount += SwapChain::MAX_FRAMES_IN_FLIGHT;
				} else if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
					combinedImageSamplerCount += SwapChain::MAX_FRAMES_IN_FLIGHT;
				}
			}, bindingTuples), ...);
		}, vulkanFlags);

		// Create pool for this specific render system
		auto builder = DixDescriptorPool::Builder(m_dixDevice)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformBufferCount)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, storageBufferCount)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount);

		m_systemDescriptorPools[info.renderSystemName] = builder.build();
	}
};

} // namespace dix

#endif // APP_CONTEXT_HPP
