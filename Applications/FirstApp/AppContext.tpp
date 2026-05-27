#ifndef APP_CONTEXT_TPP
#define APP_CONTEXT_TPP

#ifndef __clang__

namespace dix {

namespace detail {

// Returns true for any Vulkan descriptor type backed by a VkBuffer.
inline constexpr bool isBufferDescriptorType(vk::DescriptorType type) {
    return type == vk::DescriptorType::eUniformBuffer ||
           type == vk::DescriptorType::eStorageBuffer ||
           type == vk::DescriptorType::eUniformBufferDynamic ||
           type == vk::DescriptorType::eStorageBufferDynamic;
}

// Returns true for any Vulkan descriptor type backed by a VkImage / VkSampler.
inline constexpr bool isImageDescriptorType(vk::DescriptorType type) {
    return type == vk::DescriptorType::eCombinedImageSampler ||
           type == vk::DescriptorType::eSampledImage ||
           type == vk::DescriptorType::eStorageImage ||
           type == vk::DescriptorType::eInputAttachment ||
           type == vk::DescriptorType::eSampler;
}

// Maps a buffer-type descriptor to the appropriate VkBufferUsageFlags.
inline constexpr vk::BufferUsageFlags descriptorTypeToBufferUsage(
    vk::DescriptorType type) {
    switch (type) {
        case vk::DescriptorType::eStorageBuffer:
        case vk::DescriptorType::eStorageBufferDynamic:
            return vk::BufferUsageFlagBits::eStorageBuffer;
        default:
            return vk::BufferUsageFlagBits::eUniformBuffer;
    }
}

}  // namespace detail

template <typename... RenderSystems>
void AppContext<RenderSystems...>::drawFrame(
    DixCamera& camera, float frameTime,
    std::unordered_map<std::string, std::vector<GameObject>>& gameObjects,
    const glm::vec3& playerPosition) {
    auto extent = m_Window.getExtent();
    if (extent.width == 0 || extent.height == 0) return;

    AdditionalUIInfo additionalInfo{.playerPosition = playerPosition};
    if (m_uiManager) {
        m_uiManager->update(frameTime, additionalInfo);
    }

    auto commandBuffer = beginFrame();
    if (!commandBuffer) return;

    int frameIndex = getFrameIndex();

    // UI data upload (CPU-side, no render pass required)-
    // The UI system uses its own internal pipeline and does not
    // participate in the render-system descriptor set machinery,
    // so globalDescriptorSet is nullptr here.
    FrameInfo uiFrameInfo{frameIndex, frameTime, commandBuffer,
                          camera,     nullptr,   m_Window.getExtent()};
    if (m_uiManager) {
        m_uiManager->upload(uiFrameInfo);
    }

    // Compute pass — must run OUTSIDE the render pass.
    // Each system's dispatchCompute() is a no-op when the
    // system has no compute pipeline, so this is always safe.
    // Each system is also responsible for inserting its own
    // compute → graphics memory barrier inside dispatchCompute.
    std::apply(
        [&](auto&&... renderSystemDescs) {
            (renderSystemDescs.renderSystem->dispatchCompute(commandBuffer),
             ...);
        },
        m_renderSystemRegistery.getRenderSystemDescriptions());

    // Graphics pass
    beginSwapChainRenderPass(commandBuffer);

    std::apply(
        [&](auto&&... renderSystemDescs) {
            (([&](auto&& desc) {
                 const auto& renderSystemName = desc.renderSystemName;

                 FrameInfo frameInfo{
                     frameIndex,
                     frameTime,
                     commandBuffer,
                     camera,
                     m_systemDescriptorSets[renderSystemName][frameIndex],
                     m_Window.getExtent()};

                 // Update each AppContext-managed buffer for this frame.
                 // Uses if-constexpr requires-checks so that only the fields
                 // actually present on each UBO type are populated — no
                 // hardcoded assumptions about field names.
                 int uboTypeIndex = 0;
                 std::apply(
                     [&](auto&&... uboArgs) {
                         (([&](auto&& arg) {
                              using UboType =
                                  std::remove_reference_t<decltype(arg)>;
                              UboType ubo{};

                              if constexpr (requires { ubo.projectionView; }) {
                                  ubo.projectionView =
                                      camera.getProjection() * camera.getView();
                              }

                              if constexpr (requires {
                                                ubo.projection;
                                                ubo.view;
                                            }) {
                                  ubo.projection = camera.getProjection();
                                  ubo.view = camera.getView();
                              }

                              m_systemUboBuffers[renderSystemName][frameIndex]
                                                [uboTypeIndex]
                                                    ->writeToBuffer(
                                                        &ubo, sizeof(UboType));
                              m_systemUboBuffers[renderSystemName][frameIndex]
                                                [uboTypeIndex]
                                                    ->flush();
                              ++uboTypeIndex;
                          }(std::get<0>(
                               std::tuple<std::decay_t<decltype(uboArgs)>>{}))),
                          ...);
                     },
                     desc.Ubos);

                 desc.renderSystem->renderGameObjects(
                     frameInfo, gameObjects[renderSystemName]);
             }(renderSystemDescs)),
             ...);
        },
        m_renderSystemRegistery.getRenderSystemDescriptions());

    // UI render pass
    // Push constants and pipeline binding are encapsulated in
    // UIRenderer — AppContext does not know about UI internals.
    if (m_uiManager && m_uiRenderer) {
        m_uiRenderer->bindPipeline(commandBuffer);
        m_uiRenderer->uploadPushConstants(commandBuffer, m_Window.getExtent());
        m_uiManager->render(uiFrameInfo);
    }

    endSwapChainRenderPass(commandBuffer);
    endFrame();
}

template <typename... RenderSystems>
template <typename RenderSystemInfo>
void AppContext<RenderSystems...>::createSingleUbo(RenderSystemInfo&& info) {
    using RenderSystemType = std::decay_t<decltype(*info.renderSystem)>;
    constexpr auto flags = RenderSystemType::getVulkanFlags();
    constexpr size_t uboCount =
        std::tuple_size_v<std::remove_reference_t<decltype(info.Ubos)>>;

    // Build an ordered list of (binding, descriptorType) for every
    // buffer-type flag.  Index i in this list corresponds to Ubos[i].
    std::vector<std::pair<uint32_t, vk::DescriptorType>> bufferBindings;
    bufferBindings.reserve(uboCount);
    std::apply(
        [&](auto&&... flag) {
            (([&](const auto& f) {
                 if (detail::isBufferDescriptorType(std::get<1>(f))) {
                     bufferBindings.emplace_back(std::get<0>(f),
                                                 std::get<1>(f));
                 }
             })(flag),
             ...);
        },
        flags);

    m_systemUboBuffers[info.renderSystemName].resize(
        SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (auto& frameBuffers : m_systemUboBuffers[info.renderSystemName]) {
        frameBuffers.resize(uboCount);

        size_t uboTypeIndex = 0;
        std::apply(
            [&](auto&&... uboTypes) {
                (([&](auto&& uboTypeInstance) {
                     using UboType = std::decay_t<decltype(uboTypeInstance)>;

                     // Pick usage flags from the matching buffer binding, or
                     // fall back to UNIFORM if the Ubos tuple is longer than
                     // the buffer-type flags (shouldn't happen with a correct
                     // render system, but guards against mistakes).
                     vk::BufferUsageFlags usage =
                         vk::BufferUsageFlagBits::eUniformBuffer;
                     if (uboTypeIndex < bufferBindings.size()) {
                         usage = detail::descriptorTypeToBufferUsage(
                             bufferBindings[uboTypeIndex].second);
                     }

                     frameBuffers[uboTypeIndex] = std::make_unique<DixBuffer>(
                         m_dixDevice, sizeof(UboType), 1, usage,
                         vk::MemoryPropertyFlagBits::eHostVisible |
                             vk::MemoryPropertyFlagBits::eHostCoherent);
                     frameBuffers[uboTypeIndex]->map();
                     ++uboTypeIndex;
                 }(std::get<0>(
                      std::tuple<std::decay_t<decltype(uboTypes)>>{}))),
                 ...);
            },
            info.Ubos);
    }
}

template <typename... RenderSystems>
template <typename RenderSystemInfo>
void AppContext<RenderSystems...>::createSingleDescriptorSet(
    RenderSystemInfo&& info) {
    using RenderSystemType = std::decay_t<decltype(*info.renderSystem)>;
    constexpr auto flags = RenderSystemType::getVulkanFlags();

    m_systemDescriptorSets[info.renderSystemName].resize(
        SwapChain::MAX_FRAMES_IN_FLIGHT);

    // Build binding -> buffer-array-index map.
    // Buffer indices are assigned sequentially across buffer-type
    // bindings only (skipping image/sampler bindings).
    std::unordered_map<uint32_t, size_t> bindingToBufferIndex;
    size_t bufferIdx = 0;
    std::apply(
        [&](auto&&... flag) {
            (([&](const auto& f) {
                 if (detail::isBufferDescriptorType(std::get<1>(f))) {
                     bindingToBufferIndex[std::get<0>(f)] = bufferIdx++;
                 }
             })(flag),
             ...);
        },
        flags);

    vk::DescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView = m_defaultTexture.getImageView();
    imageInfo.sampler = m_defaultTexture.getSampler();

    for (size_t i = 0; i < m_systemDescriptorSets[info.renderSystemName].size();
         ++i) {
        DixDescriptorWriter writer(*m_systemSetLayouts[info.renderSystemName],
                                   *m_systemPool[info.renderSystemName]);

        std::apply(
            [&](auto&&... flag) {
                (([&](const auto& f) {
                     uint32_t binding = std::get<0>(f);
                     vk::DescriptorType type = std::get<1>(f);

                     if (detail::isBufferDescriptorType(type)) {
                         auto it = bindingToBufferIndex.find(binding);
                         if (it != bindingToBufferIndex.end()) {
                             size_t idx = it->second;
                             if (idx <
                                 m_systemUboBuffers[info.renderSystemName][i]
                                     .size()) {
                                 auto bufferInfo =
                                     m_systemUboBuffers[info.renderSystemName]
                                                       [i][idx]
                                                           ->descriptorInfo();
                                 writer.writeBuffer(binding, &bufferInfo);
                             }
                         }
                     } else if (detail::isImageDescriptorType(type)) {
                         writer.writeImage(binding, &imageInfo);
                     }
                 })(flag),
                 ...);
            },
            flags);

        if (!writer.build(m_systemDescriptorSets[info.renderSystemName][i])) {
            throw std::runtime_error(
                "Failed to build descriptor set for " +
                static_cast<std::string>(info.renderSystemName));
        }
    }
}

template <typename... RenderSystems>
template <size_t... Indices>
void AppContext<RenderSystems...>::createRenderSystemsImpl(
    std::index_sequence<Indices...>) {
    (([&]() {
         using T = std::remove_reference_t<
             decltype(*std::get<Indices>(m_renderSystemRegistery
                                             .getRenderSystemDescriptions())
                           .renderSystem)>;
         auto& renderSystemDesc = std::get<Indices>(
             m_renderSystemRegistery.getRenderSystemDescriptions());
         const auto& renderSystemName = renderSystemDesc.renderSystemName;

         renderSystemDesc.renderSystem = std::make_unique<T>(
             m_dixDevice, m_dixRenderer.getSwapChainRenderPass(),
             m_systemSetLayouts[renderSystemName]->getDescriptorSetLayout(),
             m_modelSetLayout->getDescriptorSetLayout());

         renderSystemDesc.renderSystem->setDescriptorPool(
             std::move(m_systemPool[renderSystemName]));
     })(),
     ...);
}

template <typename... RenderSystems>
void AppContext<RenderSystems...>::createDescriptorPools() {
    std::apply(
        [&](auto&&... args) {
            (createSingleDescriptorPool(std::forward<decltype(args)>(args)),
             ...);
        },
        m_renderSystemRegistery.getRenderSystemDescriptions());
}

template <typename... RenderSystems>
template <typename RenderSystemInfo>
void AppContext<RenderSystems...>::createSingleDescriptorPool(
    RenderSystemInfo&& info) {
    using RenderSystemType = std::decay_t<decltype(*info.renderSystem)>;
    constexpr auto flags = RenderSystemType::getVulkanFlags();

    auto builder = DixDescriptorPool::Builder(m_dixDevice)
                       .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT);

    // Add one pool-size entry per binding declared in the flags.
    // Duplicate types are additive in Vulkan pool size arrays.
    std::apply(
        [&](auto&&... flag) {
            (([&](const auto& f) {
                 builder.addPoolSize(std::get<1>(f),
                                     SwapChain::MAX_FRAMES_IN_FLIGHT);
             })(flag),
             ...);
        },
        flags);

    m_systemPool[info.renderSystemName] = builder.build();
}

template <typename... RenderSystems>
void AppContext<RenderSystems...>::createSystemSetLayouts() {
    auto processRenderSystem = [&](auto&& renderSystemDesc) {
        auto&& vulkanFlags = renderSystemDesc.renderSystem->getVulkanFlags();
        auto builder = DixDescriptorSetLayout::Builder(m_dixDevice);
        std::apply(
            [&](auto&&... bindingTuples) {
                (std::apply(
                     [&](auto&&... args) {
                         builder.addBinding(
                             std::forward<decltype(args)>(args)...);
                     },
                     bindingTuples),
                 ...);
            },
            vulkanFlags);
        m_systemSetLayouts[renderSystemDesc.renderSystemName] = builder.build();
    };

    std::apply([&](auto&&... args) { (processRenderSystem(args), ...); },
               m_renderSystemRegistery.getRenderSystemDescriptions());
}

template <typename... RenderSystems>
void AppContext<RenderSystems...>::createModelDescriptorResources() {
    // Upper bound on the number of distinct model textures the
    // pool can hold.  Increase if the scene has more textured models.
    static constexpr uint32_t MAX_MODEL_DESCRIPTORS = 1000;

    // Collect every image-type binding declared across all render
    // systems.  De-duplicate by binding index so the set layout has
    // exactly one entry per binding slot.
    auto layoutBuilder = DixDescriptorSetLayout::Builder(m_dixDevice);
    std::unordered_set<uint32_t> addedBindings;

    std::apply(
        [&](auto&&... desc) {
            (([&](auto&& d) {
                 using RS = std::decay_t<decltype(*d.renderSystem)>;
                 constexpr auto flags = RS::getVulkanFlags();
                 std::apply(
                     [&](auto&&... flag) {
                         (([&](const auto& f) {
                              if (detail::isImageDescriptorType(
                                      std::get<1>(f))) {
                                  uint32_t binding = std::get<0>(f);
                                  if (addedBindings.insert(binding).second) {
                                      layoutBuilder.addBinding(binding,
                                                               std::get<1>(f),
                                                               std::get<2>(f));
                                  }
                              }
                          })(flag),
                          ...);
                     },
                     flags);
             }(desc)),
             ...);
        },
        m_renderSystemRegistery.getRenderSystemDescriptions());

    m_modelSetLayout = layoutBuilder.build();

    // Build the pool from the bindings actually present in the layout.
    auto poolBuilder = DixDescriptorPool::Builder(m_dixDevice)
                           .setMaxSets(MAX_MODEL_DESCRIPTORS);

    for (auto& [binding, layoutBinding] : m_modelSetLayout->getBindings()) {
        poolBuilder.addPoolSize(layoutBinding.descriptorType,
                                MAX_MODEL_DESCRIPTORS);
    }

    m_modelDescriptorPool = poolBuilder.build();
}

template <typename... RenderSystems>
void AppContext<RenderSystems...>::createRenderSystems() {
    createRenderSystemsImpl(std::index_sequence_for<RenderSystems...>{});
}

template <typename... RenderSystems>
void AppContext<RenderSystems...>::createDescriptorSets() {
    m_defaultTexture = createDefaultTexture(m_dixDevice);
    std::apply(
        [this](auto&&... arg) {
            (createSingleDescriptorSet(std::forward<decltype(arg)>(arg)), ...);
        },
        m_renderSystemRegistery.getRenderSystemDescriptions());
}

template <typename... RenderSystems>
void AppContext<RenderSystems...>::createUBOs() {
    std::apply(
        [this](auto&&... arg) {
            (createSingleUbo(std::forward<decltype(arg)>(arg)), ...);
        },
        m_renderSystemRegistery.getRenderSystemDescriptions());
}

}  // namespace dix
#endif  // __clang__
#endif  // APP_CONTEXT_TPP