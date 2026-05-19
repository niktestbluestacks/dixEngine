#ifndef PARTICLE_RENDER_SYSTEM_HPP
#define PATRICLE_RENDER_SYSTEM_HPP

// dix
#include "Pipeline/EngineDevice/EngineDevice.hpp"
#include "Utils/Class.hpp"
#include <Rendering/RenderSystem/DixRenderSystem.hpp>
#include <Pipeline/ComputePipeline/ComputePipeline.hpp>
#include <Pipeline/Buffer/DixBuffer.hpp>
#include <tuple>

namespace dix {

struct Particle {
    alignas(16) glm::vec3 position;
    float lifetime;
    alignas(16) glm::vec3 velocity;
    float size;
    alignas(16) glm::vec4 color;
};

struct ParticleSimulationParams {
    alignas(16) glm::vec3 gravity { 0.f, -1.f, 0.f };
    float deltaTime;
    alignas(16) glm::vec3 wind { 0.f, 0.f, 0.f };
    float damping { .99f };
};

struct ParticleUbo {
    alignas(16) glm::mat4 projectionView { 1.f };
};

struct ParticlePushConstantData {
    alignas(16) glm::mat4 modelMatrix { 1.f };
};

class ParticleRenderSystem : public DixRenderSystem {
public:
    using DixRenderSystem::DixRenderSystem;
    using Ubos = std::tuple<ParticleUbo>;
    using PushConstantData = ParticlePushConstantData;

    ParticleRenderSystem(
        EngineDevice& engineDevice,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout,
        VkDescriptorSetLayout modelSetLayout
    );

    ~ParticleRenderSystem();

    static constexpr const char* Name() {
        return "ParticleRenderSystem";
    }

    static constexpr std::tuple<VulkanRenderSystemFlagType, VulkanRenderSystemFlagType> getVulkanFlags() {
        return std::make_tuple(
            VulkanRenderSystemFlagType{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT },
            VulkanRenderSystemFlagType{ 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT }
        );
    }

    DIX_DISABLE_COPY(ParticleRenderSystem)

    void updateParticles(float deltaTime);
    void createParticleEmitter(glm::vec3 position, uint32_t count);
    void dispatchCompute(VkCommandBuffer commandBuffer, uint32_t particleCount);

private:
    void createComputePipeline(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout modelSetLayout);
    void setupDescriptors();
    void bindBuffers(VkCommandBuffer commandBuffer);

    std::unique_ptr<ComputePipeline> m_computePipeline;
    VkPipelineLayout m_computePipelineLayout{ VK_NULL_HANDLE };
    std::unique_ptr<DixDescriptorSetLayout> m_computeSetLayout;

    std::unique_ptr<DixBuffer> m_particleBuffer;
    std::unique_ptr<DixBuffer> m_simulationParamsBuffer;

    ParticleSimulationParams m_simParams{};
    uint32_t m_particleCount{ 0 };
    static constexpr uint32_t MAX_PARTICLES = 10000;
};  // ParticleRenderSystem
}   // namespace dix

#endif // PARTICLE_RENDER_SYSTEM_HPP