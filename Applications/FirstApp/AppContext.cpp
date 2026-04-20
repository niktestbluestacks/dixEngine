// dix
#include <FirstApp/AppContext.hpp>

#include <Utils/Converter.hpp>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <stdexcept>

namespace dix {

struct GlobalUbo {
    alignas(16) glm::mat4 projectionView{ 1.f };
    alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
};

AppContext::AppContext(int width, int height, const std::string& title) :
    m_Window{ width, height, title },
    m_dixDevice{ m_Window },
    m_dixRenderer{ m_Window, m_dixDevice } {
    initialize();
}

AppContext::~AppContext() {
    // resources owned by unique_ptr will be destroyed automatically
}

void AppContext::initialize() {
    createDescriptorPool();
    createUBOs();
    m_globalSetLayout = DixDescriptorSetLayout::Builder(m_dixDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .build();
    createDescriptorSets();
    createRenderSystem();
}

void AppContext::createDescriptorPool() {
    m_globalPool = DixDescriptorPool::Builder(m_dixDevice)
        .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();
}

void AppContext::createUBOs() {
    m_uboBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (auto& buf : m_uboBuffers) {
        buf = std::make_unique<DixBuffer>(
            m_dixDevice,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        buf->map();
    }
}

void AppContext::createDescriptorSets() {
    m_globalDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < m_globalDescriptorSets.size(); ++i) {
        auto bufferInfo = m_uboBuffers[i]->descriptorInfo();
        DixDescriptorWriter(*m_globalSetLayout, *m_globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(m_globalDescriptorSets[i]);
    }
}

void AppContext::createRenderSystem() {
    m_simpleRenderSystem = std::make_unique<SimpleRenderSystem>(
        m_dixDevice,
        m_dixRenderer.getSwapChainRenderPass(),
        m_globalSetLayout->getDescriptorSetLayout()
    );
}

void AppContext::drawFrame(DixCamera& camera, float frameTime, const std::vector<GameObject>& gameObjects) {
    if (auto commandBuffer = beginFrame()) {
        int frameIndex = getFrameIndex();
        FrameInfo frameInfo{
            frameIndex,
            frameTime,
            commandBuffer,
            camera,
            m_globalDescriptorSets[frameIndex]
        };

        // update global ubo for this frame
        GlobalUbo ubo{};
        ubo.projectionView = camera.getProjection() * camera.getView();
        m_uboBuffers[frameIndex]->writeToIndex(&ubo, 0);
        m_uboBuffers[frameIndex]->flush();

        // render
        beginSwapChainRenderPass(commandBuffer);
        m_simpleRenderSystem->renderGameObjects(const_cast<FrameInfo&>(frameInfo), const_cast<std::vector<GameObject>&>(gameObjects));
        endSwapChainRenderPass(commandBuffer);
        endFrame();
    }
}

} // namespace dix
