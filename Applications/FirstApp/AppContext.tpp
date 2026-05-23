#ifndef APP_CONTEXT_TPP
#define APP_CONTEXT_TPP

namespace dix {

template <typename... RenderSystems>
void AppContext<RenderSystems...>::drawFrame(
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
                &screenSize);
            m_uiManager->render(uiFrameInfo);
        }
        endSwapChainRenderPass(commandBuffer);
        endFrame();
    }
}

template <typename... RenderSystems> template <typename RenderSystemInfo>
void AppContext<RenderSystems...>::createSingleUbo(RenderSystemInfo&& info) {
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

template <typename... RenderSystems>
template <typename RenderSystemInfo>
void AppContext<RenderSystems...>::createSingleDescriptorSet(RenderSystemInfo&& info) {
    using RenderSystemType = std::decay_t<decltype(*info.renderSystem)>;
    constexpr auto flags = RenderSystemType::getVulkanFlags();
    
    m_systemDescriptorSets[info.renderSystemName].resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    // Prepare a default image info just in case a binding requires it (fallback)
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_defaultTexture.getImageView();
    imageInfo.sampler = m_defaultTexture.getSampler();

    for (size_t i = 0; i < m_systemDescriptorSets[info.renderSystemName].size(); ++i) {
        DixDescriptorWriter writer(*m_systemSetLayouts[info.renderSystemName], *m_systemPool[info.renderSystemName]);

        // Iterate over the compile-time flags to write descriptors dynamically
        std::apply([&](auto&&... flag) {
            (([&](const auto& f) {
                uint32_t binding = std::get<0>(f);
                VkDescriptorType type = std::get<1>(f);

                if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
                    // Assume buffer index matches binding index for simplicity, 
                    // or map binding -> buffer index if your strategy differs
                    if (binding < m_systemUboBuffers[info.renderSystemName][i].size()) {
                        auto bufferInfo = m_systemUboBuffers[info.renderSystemName][i][binding]->descriptorInfo();
                        writer.writeBuffer(binding, &bufferInfo);
                    }
                } else if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) {
                    writer.writeImage(binding, &imageInfo);
                }
            })(flag), ...);
        }, flags);

        if (!writer.build(m_systemDescriptorSets[info.renderSystemName][i])) {
            throw std::runtime_error("Failed to build descriptor set for " + static_cast <std::string>(info.renderSystemName));
        }
    }
}

template <typename... RenderSystems> template<size_t... Indices>
void AppContext<RenderSystems...>::createRenderSystemsImpl(std::index_sequence<Indices...>) {
    (([&]() {
        using T = std::remove_reference_t<decltype(*std::get<Indices>(m_renderSystemRegistery.getRenderSystemDescriptions()).renderSystem)>;
        auto& renderSystemDesc = std::get<Indices>(m_renderSystemRegistery.getRenderSystemDescriptions());
        const auto& renderSystemName = renderSystemDesc.renderSystemName;

        renderSystemDesc.renderSystem = std::make_unique<T>(
            m_dixDevice,
            m_dixRenderer.getSwapChainRenderPass(),
            m_systemSetLayouts[renderSystemName]->getDescriptorSetLayout(),
            m_modelSetLayout->getDescriptorSetLayout()
        );

        // Transfer ownership of the descriptor pool to the render system
        renderSystemDesc.renderSystem->setDescriptorPool(std::move(m_systemPool[renderSystemName]));
    })(), ...);
}
	
template <typename... RenderSystems>
void AppContext<RenderSystems...>::createDescriptorPools() {
    std::apply([&](auto&&... args) {
        (createSingleDescriptorPool(std::forward<decltype(args)>(args)), ...);
    }, m_renderSystemRegistery.getRenderSystemDescriptions());
}

template <typename... RenderSystems> template <typename RenderSystemInfo>
void AppContext<RenderSystems...>::createSingleDescriptorPool(RenderSystemInfo&& info) {
    m_systemPool[info.renderSystemName] = DixDescriptorPool::Builder(m_dixDevice)
        .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();
}

template <typename... RenderSystems>
void AppContext<RenderSystems...>::createSystemSetLayouts() {
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

template <typename... RenderSystems>
void AppContext<RenderSystems...>::createModelDescriptorResources() {
    m_modelSetLayout = DixDescriptorSetLayout::Builder(m_dixDevice)
    .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
    .build();

    // Create a large pool for model descriptor sets
    m_modelDescriptorPool = DixDescriptorPool::Builder(m_dixDevice)
        .setMaxSets(1000)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
        .build();
}

template <typename... RenderSystems>
void AppContext<RenderSystems...>::createRenderSystems() {
    createRenderSystemsImpl(std::index_sequence_for<RenderSystems...>{});
}

template <typename... RenderSystems>
void AppContext<RenderSystems...>::createDescriptorSets() {
    m_defaultTexture = createDefaultTexture(m_dixDevice);
    std::apply([this](auto&&... arg) {
        (createSingleDescriptorSet(std::forward<decltype(arg)>(arg)), ...);
    }, m_renderSystemRegistery.getRenderSystemDescriptions());
}

template <typename... RenderSystems>
void AppContext<RenderSystems...>::createUBOs() {
    std::apply([this](auto&&... arg) {
        (createSingleUbo(std::forward<decltype(arg)>(arg)), ...);
    }, m_renderSystemRegistery.getRenderSystemDescriptions());	
}
}   // namespace dix
#endif // APP_CONTEXT_TPP