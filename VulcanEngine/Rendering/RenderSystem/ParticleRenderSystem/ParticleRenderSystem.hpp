#ifndef PARTICLE_RENDER_SYSTEM_HPP
#define PARTICLE_RENDER_SYSTEM_HPP

// dix
#include <Rendering/RenderSystem/DixRenderSystem.hpp>
#include <Pipeline/Buffer/DixBuffer.hpp>

// std
#include <tuple>

namespace dix {

struct Particle {
    alignas(16) glm::vec4 positionLifetime;
    alignas(16) glm::vec4 velocitySize;
    alignas(16) glm::vec4 color;
    alignas(16) glm::vec4 initPosLife;
    alignas(16) glm::vec3 initVelocity;
};

struct ParticleSimulationParams {
    alignas(16) glm::vec4 gravityDeltaTime { 0.f, -.1f, 0.f, 0.f };
    alignas(16) glm::vec4 windDamping { 0.f, 0.f, 0.f, .999f };
    alignas(16) glm::vec4 particlesPosLife { 0.f, 0.f, 0.f, 100.f };
};

struct ParticleUbo {
    alignas(16) glm::mat4 projectionView{ 1.f };
};

struct ParticlePushConstantData {
    alignas(16) glm::mat4 modelMatrix{ 1.f };
};

using ParticleRenderSystemBindings = std::tuple<
    UniformBinding<ParticleUbo, 0, VK_SHADER_STAGE_VERTEX_BIT>
>;

class ParticleRenderSystem:
    public DixRenderSystem,
    public RenderSystemTraits<ParticleRenderSystemBindings> {
public:
    using Ubos = std::tuple<ParticleUbo>;
    using PushConstantData = ParticlePushConstantData;

    ParticleRenderSystem(
        EngineDevice& engineDevice,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout,
        VkDescriptorSetLayout modelSetLayout
    );
    ~ParticleRenderSystem() override = default;

    static constexpr const char* Name() { return "ParticleRenderSystem"; }

    DIX_DISABLE_COPY(ParticleRenderSystem)

    // Particle management
    void updateParticles(float deltaTime);
    void createParticleEmitter(glm::vec3 position, uint32_t count);

    // Dispatches the compute shader. Call outside a render pass.
    void dispatchCompute(VkCommandBuffer commandBuffer) override;

    void renderGameObjects(FrameInfo& frameInfo, std::vector<GameObject>& gameObjects) override;

protected:
    // Writes m_particleBuffer and m_simulationParamsBuffer into
    // the compute descriptor set allocated by the base class.
    void buildComputeDescriptors() override;

private:
    void bindBuffers(VkCommandBuffer commandBuffer) const;

    // Particle storage buffer: [uint32_t count | Particle[MAX_PARTICLES]]
    std::unique_ptr<DixBuffer> m_particleBuffer;
    // UBO for simulation params fed to the compute shader
    std::unique_ptr<DixBuffer> m_simulationParamsBuffer;

    ParticleSimulationParams m_simParams{};
    uint32_t m_particleCount{ 0 };
    static constexpr uint32_t MAX_PARTICLES = 1'000'000;
};

}   // namespace dix
#endif // PARTICLE_RENDER_SYSTEM_HPP
