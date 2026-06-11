#ifndef PARTICLE_RENDER_SYSTEM_HPP
#define PARTICLE_RENDER_SYSTEM_HPP

// dix
#include <Model/GameObject/ParticleEmitter.hpp>
#include <Pipeline/Buffer/DixBuffer.hpp>
#include <Rendering/RenderSystem/DixRenderSystem.hpp>

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

struct ParticleUbo {
    alignas(16) glm::mat4 projectionView{1.f};
};

struct ParticlePushConstantData {
    alignas(16) glm::mat4 modelMatrix{1.f};
};

using ParticleRenderSystemBindings = std::tuple<
    UniformBinding<ParticleUbo, 0, vk::ShaderStageFlagBits::eVertex>>;

class ParticleRenderSystem
    : public DixRenderSystem,
      public RenderSystemTraits<ParticleRenderSystemBindings> {
   public:
    using PushConstantData = ParticlePushConstantData;

    ParticleRenderSystem(EngineDevice& engineDevice, vk::RenderPass renderPass,
                         vk::DescriptorSetLayout globalSetLayout,
                         vk::DescriptorSetLayout modelSetLayout);
    ~ParticleRenderSystem() override = default;

    static constexpr const char* Name() { return "ParticleRenderSystem"; }

    DIX_DISABLE_COPY(ParticleRenderSystem)

    // Particle management
    void updateParticles(
        float deltaTime,
        const std::vector<std::shared_ptr<ParticleEmitter>>& particleEmitters);
    std::shared_ptr<ParticleEmitter> createParticleEmitter(glm::vec3 position,
                                                           uint32_t count);

    // Dispatches the compute shader. Call outside a render pass.
    void dispatchCompute(
        vk::CommandBuffer commandBuffer,
        std::vector<std::shared_ptr<GameObject>>& obj) override;
    void renderGameObjects(
        FrameInfo& frameInfo,
        std::vector<std::shared_ptr<GameObject>>& gameObjects) override;
    static constexpr size_t MAX_PARTICLE_EMITTERS = 100;

   protected:
    // Writes m_particleBuffer and m_simulationParamsBuffer into
    // the compute descriptor set allocated by the base class.
    void buildComputeDescriptors(ParticleEmitter& obj);

   private:
    void bindBuffers(vk::CommandBuffer commandBuffer,
                     std::shared_ptr<ParticleEmitter>& obj) const;
};

}  // namespace dix
#endif  // PARTICLE_RENDER_SYSTEM_HPP