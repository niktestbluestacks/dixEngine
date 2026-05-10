// dix
#include <FirstApp/AppContext.hpp>

#include <UI/DixUIElement.hpp>
#include <Utils/Converter.hpp>
#include <memory>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace dix {

AppContext::AppContext(int width, int height, const std::string& title) :
    m_Window{ width, height, title },
    m_dixDevice{ m_Window },
    m_dixRenderer{ m_Window, m_dixDevice } {
    initialize();
    m_uiManager = std::make_unique<dix::UIManager>();
    m_uiRenderer = std::make_unique<dix::UIRenderer>(m_dixDevice, m_dixRenderer.getSwapChainRenderPass());
}

AppContext::~AppContext() {
    // resources owned by unique_ptr will be destroyed automatically
}

void AppContext::initialize() {
    declareRenderSystems();
    createDescriptorPool();
    createUBOs();
    createSystemSetLayouts();
    createDescriptorSets();
    createModelDescriptorResources();
    createRenderSystem();
}

void AppContext::declareRenderSystems() {
    DIX_RSR.declareRenderSystem<SimpleUbo>("SimpleRenderSystem");
}

void AppContext::createDescriptorPool() {
    // m_globalPool = DixDescriptorPool::Builder(m_dixDevice)
    //     .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
    //     .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
    //     .build();
    m_globalPool = DixDescriptorPool::Builder(m_dixDevice)
        .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();
}

void AppContext::createUBOs() {
    for (auto & [renderSystemName, renderInfo] : DIX_RSR.getRenderSystems()) {
        m_systemUboBuffers[renderSystemName].resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto& buf : m_systemUboBuffers[renderSystemName]) {
        auto uboInfo = DIX_RSR.getUboTypeInfo(renderSystemName);
        assert(uboInfo.size % 16 == 0 && "GlobalUbo size must be a alligned to 16 bytes!");
        buf = std::make_unique<DixBuffer>(
            m_dixDevice,
            uboInfo.size,
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        buf->map();
        }
    }
}

void AppContext::createSystemSetLayouts() {
    // All of layouts must be created manually
    m_systemSetLayouts["SimpleRenderSystem"] = DixDescriptorSetLayout::Builder(m_dixDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();
}

void AppContext::createDescriptorSets() {
    // Ensure we have a valid default texture to bind for models that don't provide one.
    m_defaultTexture = createDefaultTexture(m_dixDevice);
    for (auto& [renderSystemName, renderInfo] : DIX_RSR.getRenderSystems()) {
        m_systemDescriptorSets[renderSystemName].resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_defaultTexture.getImageView();
        imageInfo.sampler = m_defaultTexture.getSampler();

            for (size_t i = 0; i < m_systemDescriptorSets[renderSystemName].size(); ++i) {
                auto bufferInfo = m_systemUboBuffers[renderSystemName][i]->descriptorInfo();
                DixDescriptorWriter(*m_systemSetLayouts[renderSystemName], *m_globalPool)
                    .writeBuffer(0, &bufferInfo)
                    .writeImage(1, &imageInfo)
                    .build(m_systemDescriptorSets[renderSystemName][i]);
            }
    }
}

void AppContext::createRenderSystem() {
    DIX_RSR.registerRenderSystem<SimpleRenderSystem, SimpleUbo>("SimpleRenderSystem", {
        m_dixDevice,
        m_dixRenderer.getSwapChainRenderPass(),
        m_systemSetLayouts["SimpleRenderSystem"]->getDescriptorSetLayout(),
        m_modelSetLayout->getDescriptorSetLayout()
    });
}

void AppContext::createModelDescriptorResources() {
    // Create descriptor set layout for per-model textures (binding 1 only)
    m_modelSetLayout = DixDescriptorSetLayout::Builder(m_dixDevice)
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    // Create a large pool for model descriptor sets
    m_modelDescriptorPool = DixDescriptorPool::Builder(m_dixDevice)
        .setMaxSets(1000)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
        .build();
}
void AppContext::drawFrame(
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
    for (const auto &[renderSystemName, renderInfo] : DIX_RSR.getRenderSystems()) {
        const auto& [renderSystem, uboInfo] = renderInfo;
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
            SimpleRenderSystem::GlobalUbo ubo{};
            ubo.projectionView = camera.getProjection() * camera.getView();
            m_systemUboBuffers[renderSystemName][frameIndex]->writeToIndex(&ubo, 0);
            m_systemUboBuffers[renderSystemName][frameIndex]->flush();

            // UI was already updated before acquiring the swapchain image

            // render
            beginSwapChainRenderPass(commandBuffer);
            DIX_RSR.getRenderSystem(renderSystemName)->renderGameObjects(frameInfo, gameObjects[renderSystemName]);
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
}

void AppContext::addUIElement(std::unique_ptr<DixUIElement> element) {
    if (m_uiManager) {
        m_uiManager->addElement(std::move(element));
    }
}

// void AppContext::addGameObject(std::unique_ptr<GameObject>) {
//     if (m_gameObjects) {
//         m_gameObjects->addObject(std::move(object));
//     }
// }

} // namespace dix
