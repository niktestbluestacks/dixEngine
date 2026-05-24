// dix
#include <Rendering/RenderSystem/DixRenderSystem.hpp>
#include <Utils/Converter.hpp>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <cassert>
#include <stdexcept>

namespace dix {

DixRenderSystem::DixRenderSystem(
    EngineDevice& engineDevice,
    VkRenderPass renderPass,
    VkDescriptorSetLayout globalSetLayout,
    VkDescriptorSetLayout modelSetLayout,
    DixRenderSystemConfig config)
    : m_dixDevice{ engineDevice }
    , m_config{ std::move(config) }
{
    assert(m_config.pushConstantSize <= MAX_PUSH_CONSTANT_BYTES &&
           "Push constant block exceeds MAX_PUSH_CONSTANT_BYTES (256)");
    assert(m_config.transformGameObject &&
           "DixRenderSystemConfig::transformGameObject must not be null");

    // Build the graphics pipeline directly (non-virtual — avoids the
    // classic "calling virtuals in a constructor" problem while still
    // letting subclasses override for truly custom pipelines).
    DixRenderSystem::createPipelineLayout(globalSetLayout, modelSetLayout);
    DixRenderSystem::createPipeline(renderPass);

    // If a compute config was supplied, wire up everything the base can
    // do automatically.  buildComputeDescriptors() is intentionally NOT
    // called here — the subclass must call it after its data buffers exist.
    if (m_config.compute) {
        initComputeFromConfig(*m_config.compute);
    }
}

DixRenderSystem::~DixRenderSystem() {
    if (m_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_dixDevice.device(), m_pipelineLayout, nullptr);
    }
    if (m_computePipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_dixDevice.device(), m_computePipelineLayout, nullptr);
    }
}

void DixRenderSystem::createPipelineLayout(
    VkDescriptorSetLayout globalSetLayout,
    VkDescriptorSetLayout modelSetLayout)
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = m_config.pushConstantStages;
    pushConstantRange.offset = 0;
    pushConstantRange.size = m_config.pushConstantSize;

    std::vector<VkDescriptorSetLayout> setLayouts{ globalSetLayout, modelSetLayout };

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    layoutInfo.pSetLayouts = setLayouts.data();
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(m_dixDevice.device(), &layoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("DixRenderSystem: failed to create pipeline layout");
    }
}

void DixRenderSystem::createPipeline(VkRenderPass renderPass) {
    assert(m_pipelineLayout != VK_NULL_HANDLE &&
           "createPipelineLayout must be called before createPipeline");

    PipelineConfigInfo cfg{};
    Pipeline::defaultPipelineConfigInfo(cfg);

    cfg.inputAssemblyInfo.topology = m_config.topology;
    cfg.renderPass = renderPass;
    cfg.pipelineLayout = m_pipelineLayout;

    if (!m_config.vertexBindings.empty()) {
        cfg.vertexBindingDescriptions = m_config.vertexBindings;
    }
    if (!m_config.vertexAttributes.empty()) {
        cfg.vertexAttributeDescriptions = m_config.vertexAttributes;
    }

    m_pipeline = std::make_unique<Pipeline>(
        m_dixDevice,
        toShaderPath(m_config.vertShaderPath),
        toShaderPath(m_config.fragShaderPath),
        cfg
    );
}

void DixRenderSystem::renderGameObjects(
    FrameInfo& frameInfo,
    std::vector<GameObject>& gameObjects) const
{
    m_pipeline->bind(frameInfo.commandBuffer);

    for (auto& obj : gameObjects) {
        // Use a fixed-size stack buffer — avoids heap allocation per object
        // while supporting any push-constant size up to 256 bytes.
        std::array<std::byte, MAX_PUSH_CONSTANT_BYTES> pushBuffer{};
        m_config.transformGameObject(pushBuffer.data(), obj);

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            m_pipelineLayout,
            m_config.pushConstantStages,
            0,
            m_config.pushConstantSize,
            pushBuffer.data());

        std::array<VkDescriptorSet, 2> descriptorSets{
            frameInfo.globalDescriptorSet,
            VK_NULL_HANDLE
        };
        if (obj.model) {
            descriptorSets[1] = obj.model->getDescriptorSet();
        }

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            static_cast<uint32_t>(descriptorSets.size()),
            descriptorSets.data(),
            0, nullptr);

        if (obj.model) {
            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }
}

void DixRenderSystem::initComputeFromConfig(const ComputePipelineConfig& cc) {
    // Build the descriptor-set layout from the declared bindings.
    auto builder = DixDescriptorSetLayout::Builder(m_dixDevice);
    for (auto& [binding, type, stages] : cc.bindings) {
        builder.addBinding(binding, type, stages);
    }
    initComputeLayout(builder.build(), cc.pushRanges);
    initComputePipeline(cc.shaderPath);
    initComputeDescriptorPool(cc.descriptorPoolMaxSets);
    // buildComputeDescriptors() is deferred to the subclass constructor.
}

void DixRenderSystem::initComputeLayout(
    std::unique_ptr<DixDescriptorSetLayout> setLayout,
    const std::vector<VkPushConstantRange>& pushRanges)
{
    m_computeSetLayout = std::move(setLayout);

    VkDescriptorSetLayout vkLayout = m_computeSetLayout->getDescriptorSetLayout();

    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.setLayoutCount = 1;
    info.pSetLayouts = &vkLayout;
    info.pushConstantRangeCount = static_cast<uint32_t>(pushRanges.size());
    info.pPushConstantRanges = pushRanges.empty() ? nullptr : pushRanges.data();

    if (vkCreatePipelineLayout(m_dixDevice.device(), &info, nullptr, &m_computePipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("DixRenderSystem: failed to create compute pipeline layout");
    }
}

void DixRenderSystem::initComputePipeline(const std::string& compShaderPath) {
    assert(m_computePipelineLayout != VK_NULL_HANDLE &&
           "initComputeLayout must be called before initComputePipeline");

    ComputePipelineConfigInfo configInfo{ m_computePipelineLayout };
    m_computePipeline = std::make_unique<ComputePipeline>(
        m_dixDevice,
        toShaderPath(compShaderPath),
        configInfo
    );
}

void DixRenderSystem::initComputeDescriptorPool(uint32_t maxSets) {
    assert(m_computeSetLayout &&
           "initComputeLayout must be called before initComputeDescriptorPool");

    auto builder = DixDescriptorPool::Builder(m_dixDevice).setMaxSets(maxSets);
    for (auto& [binding, layoutBinding] : m_computeSetLayout->getBindings()) {
        builder.addPoolSize(layoutBinding.descriptorType, maxSets);
    }
    m_computeDescriptorPool = builder.build();
}

}   // namespace dix
