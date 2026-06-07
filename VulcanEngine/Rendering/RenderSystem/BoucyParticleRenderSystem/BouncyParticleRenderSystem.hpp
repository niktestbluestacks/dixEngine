#ifndef BOUNCY_PARTICLE_RENDER_SYSTEM_HPP
#define BOUNCY_PARTICLE_RENDER_SYSTEM_HPP

// dix
#include <Pipeline/Buffer/DixBuffer.hpp>
#include <Rendering/RenderSystem/DixRenderSystem.hpp>

// std
#include <tuple>

namespace dix {

struct BouncyParticle {
    alignas(16) glm::vec4 positionLifetime;
    alignas(16) glm::vec4 velocitySize;
    alignas(16) glm::vec4 color;
    alignas(16) glm::vec4 initPosLife;
    alignas(16) glm::vec3 initVelocity;
};

struct BouncyParticleSimulationParams {
    alignas(16) glm::vec4 gravityDeltaTime{0.f, -1.f, 0.f, 0.f};
    alignas(16) glm::vec4 windDamping{0.f, 0.f, 0.f, .999f};
    alignas(16) glm::vec4 particlesPosLife{0.f, 0.f, 0.f, 100.f};
};

struct BouncyParticleUbo {
    alignas(16) glm::mat4 projectionView{1.f};
};

struct BouncyParticlePushConstantData {
    alignas(16) glm::mat4 modelMatrix{1.f};
};

using BouncyParticleRenderSystemBindings = std::tuple<
    UniformBinding<BouncyParticleUbo, 0, vk::ShaderStageFlagBits::eVertex> >;

class BouncyParticleRenderSystem
    : public DixRenderSystem,
      public RenderSystemTraits<BouncyParticleRenderSystemBindings> {
   public:
    using PushConstantData = BouncyParticlePushConstantData;

    BouncyParticleRenderSystem(EngineDevice& engineDevice,
                               vk::RenderPass renderPass,
                               vk::DescriptorSetLayout globalSetLayout,
                               vk::DescriptorSetLayout modelSetLayout);
    ~BouncyParticleRenderSystem() override = default;

    static constexpr const char* Name() { return "BouncyParticleRenderSystem"; }

    DIX_DISABLE_COPY(BouncyParticleRenderSystem)

    // BouncyParticle management
    void updateParticles(float deltaTime);
    GameObject createParticleEmitter(glm::vec3 position, uint32_t count);

    // Dispatches the compute shader. Call outside a render pass.
    void dispatchCompute(vk::CommandBuffer commandBuffer) override;

    void renderGameObjects(FrameInfo& frameInfo,
                           std::vector<GameObject>& gameObjects) override;

   protected:
    // Writes m_particleBuffer and m_simulationParamsBuffer into
    // the compute descriptor set allocated by the base class.
    void buildComputeDescriptors() override;

   private:
    void bindBuffers(vk::CommandBuffer commandBuffer) const;

    // BouncyParticle storage buffer: [uint32_t count |
    // BouncyParticle[MAX_PARTICLES]]
    std::unique_ptr<DixBuffer> m_particleBuffer;
    // UBO for simulation params fed to the compute shader
    std::unique_ptr<DixBuffer> m_simulationParamsBuffer;

    BouncyParticleSimulationParams m_simParams{};
    uint32_t m_particleCount{0};
    static constexpr uint32_t MAX_PARTICLES = 1'000'000;
};

}  // namespace dix
#endif  // BOUNCY_PARTICLE_RENDER_SYSTEM_HPP