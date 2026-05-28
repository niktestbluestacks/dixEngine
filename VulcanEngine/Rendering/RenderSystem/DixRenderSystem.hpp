#ifndef DIX_RENDER_SYSTEM_HPP
#define DIX_RENDER_SYSTEM_HPP

// dix
#include <DixCamera/DixCamera.hpp>
#include <Model/GameObject/GameObject.hpp>
#include <Pipeline/ComputePipeline/ComputePipeline.hpp>
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/Pipeline/Pipeline.hpp>
#include <Utils/Class.hpp>
#include <Utils/FrameInfo.hpp>

// std
#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace dix {

using VulkanRenderSystemFlagType =
    std::tuple<uint32_t, vk::DescriptorType, vk::ShaderStageFlags>;

struct BasePushConstantData {
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
};

// ComputePipelineConfig
//
// Optional block inside DixRenderSystemConfig. When present,
// the base constructor automatically creates the compute
// descriptor-set layout, pipeline layout, compute pipeline,
// and descriptor pool.
//
// The subclass is still responsible for calling
// buildComputeDescriptors() after its own data buffers exist.
struct ComputePipelineConfig {
    // Path to the compiled .comp.spv, relative to the shader dir.
    std::string shaderPath;

    // Bindings for the compute descriptor set layout.
    // Each entry: { binding, vk::DescriptorType, vk::ShaderStageFlags }
    std::vector<VulkanRenderSystemFlagType> bindings;

    // Optional push-constant ranges for the compute pipeline layout.
    std::vector<vk::PushConstantRange> pushRanges = {};

    // How many descriptor sets the pool can allocate.
    uint32_t descriptorPoolMaxSets = 1;
};

// DixRenderSystemConfig
//
// Pass one of these to the DixRenderSystem constructor.
// The base constructor uses it to build both the graphics
// pipeline (and optionally the compute pipeline) automatically,
// so subclasses never need to call createPipelineLayout() or
// createPipeline() manually.
struct DixRenderSystemConfig {
    // Shader binary paths, relative to the shader directory.
    std::string vertShaderPath;
    std::string fragShaderPath;

    // Primitive topology. Defaults to the standard triangle list.
    vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;

    // Optional custom vertex input.  When empty the pipeline is
    // created with no vertex bindings (e.g. for full-screen quads
    // or procedural geometry driven by a storage buffer).
    std::vector<vk::VertexInputBindingDescription> vertexBindings;
    std::vector<vk::VertexInputAttributeDescription> vertexAttributes;

    // Per-object push-constant writer.
    // Signature: (pointer to push-constant block, game object)
    std::function<void(void*, GameObject&, FrameInfo&)> transformGameObject;

    // Size (bytes) of the push-constant block for this system.
    // Defaults to sizeof(BasePushConstantData).
    uint32_t pushConstantSize = sizeof(BasePushConstantData);

    // Pipeline stages that receive the push constants.
    vk::ShaderStageFlags pushConstantStages =
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

    // When set, the base constructor also sets up the compute pipeline.
    // The subclass must still call buildComputeDescriptors() after
    // creating its data buffers.
    std::optional<ComputePipelineConfig> compute = std::nullopt;
    PipelineConfigInfo pipelineConfigInfo{};
};

class DixRenderSystem {
   public:
    using PushConstantData = BasePushConstantData;

    DixRenderSystem(EngineDevice& engineDevice, vk::RenderPass renderPass,
                    vk::DescriptorSetLayout globalSetLayout,
                    vk::DescriptorSetLayout modelSetLayout,
                    DixRenderSystemConfig config);
    virtual ~DixRenderSystem();

    DIX_DISABLE_COPY(DixRenderSystem)

    virtual void renderGameObjects(FrameInfo& frameInfo,
                                   std::vector<GameObject>& gameObjects) const;

    virtual void renderGameObjects(FrameInfo& frameInfo,
                                   std::vector<GameObject>& gameObjects);

    void setDescriptorPool(std::unique_ptr<DixDescriptorPool> pool) {
        m_descriptorPool = std::move(pool);
    }
    DixDescriptorPool& getDescriptorPool() { return *m_descriptorPool; }
    const DixDescriptorPool& getDescriptorPool() const {
        return *m_descriptorPool;
    }

    // Optional compute pipeline
    bool hasComputePipeline() const noexcept {
        return m_computePipeline != nullptr;
    }

    // Override in subclasses that use compute.
    // Must be called outside a render pass (before or after graphics).
    virtual void dispatchCompute(vk::CommandBuffer /*commandBuffer*/) {}

   protected:
    // Override hooks for advanced pipeline customisation.
    // In the vast majority of cases DixRenderSystemConfig is
    // sufficient and these do NOT need to be overridden.
    virtual void createPipelineLayout(vk::DescriptorSetLayout globalSetLayout,
                                      vk::DescriptorSetLayout modelSetLayout);
    virtual void createPipeline(vk::RenderPass renderPass);

    virtual void buildComputeDescriptors() {}

    DixRenderSystemConfig m_config;

    EngineDevice& m_dixDevice;
    std::unique_ptr<Pipeline> m_pipeline;
    vk::PipelineLayout m_pipelineLayout{};

    std::unique_ptr<DixDescriptorPool> m_descriptorPool;

    static constexpr uint32_t MAX_PUSH_CONSTANT_BYTES = 256;

    std::unique_ptr<ComputePipeline> m_computePipeline;
    vk::PipelineLayout m_computePipelineLayout{};
    std::unique_ptr<DixDescriptorSetLayout> m_computeSetLayout;
    std::unique_ptr<DixDescriptorPool> m_computeDescriptorPool;
    vk::DescriptorSet m_computeDescriptorSet{};

   private:
    // Internal helpers called by the constructor.
    void initComputeFromConfig(const ComputePipelineConfig& cc);
    void initComputeLayout(
        std::unique_ptr<DixDescriptorSetLayout> setLayout,
        const std::vector<vk::PushConstantRange>& pushRanges = {});
    void initComputePipeline(const std::string& compShaderPath);
    void initComputeDescriptorPool(uint32_t maxSets = 1);
};

}  // namespace dix

#include <Rendering/RenderSystem/RenderSystemTraits.hpp>
#endif  // DIX_RENDER_SYSTEM_HPP