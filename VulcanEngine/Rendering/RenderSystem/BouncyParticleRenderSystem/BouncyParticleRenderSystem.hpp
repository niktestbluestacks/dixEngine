// #ifndef BOUNCY_PARTICLE_RENDER_SYSTEM_HPP
// #define BOUNCY_PARTICLE_RENDER_SYSTEM_HPP

// // dix
// #include <Pipeline/Buffer/DixBuffer.hpp>
// #include <Rendering/RenderSystem/DixRenderSystem.hpp>
// #include <Model/GameObject/BouncyParticleEmitter.hpp>

// // std
// #include <tuple>

// namespace dix {

// struct BouncyBouncyParticle {
//     alignas(16) glm::vec4 positionLifetime;
//     alignas(16) glm::vec4 velocitySize;
//     alignas(16) glm::vec4 color;
//     alignas(16) glm::vec4 initPosLife;
//     alignas(16) glm::vec3 initVelocity;
// };

// struct BouncyBouncyParticleUbo {
//     alignas(16) glm::mat4 projectionView{1.f};
// };

// struct BouncyBouncyParticlePushConstantData {
//     alignas(16) glm::mat4 modelMatrix{1.f};
// };

// using BouncyBouncyParticleRenderSystemBindings = std::tuple<
//     UniformBinding<BouncyBouncyParticleUbo, 0,
//     vk::ShaderStageFlagBits::eVertex> >;

// class BouncyBouncyParticleRenderSystem
//     : public DixRenderSystem,
//       public RenderSystemTraits<BouncyBouncyParticleRenderSystemBindings> {
//    public:
//     using PushConstantData = BouncyBouncyParticlePushConstantData;

//     BouncyBouncyParticleRenderSystem(EngineDevice& engineDevice,
//                                vk::RenderPass renderPass,
//                                vk::DescriptorSetLayout globalSetLayout,
//                                vk::DescriptorSetLayout modelSetLayout);
//     ~BouncyBouncyParticleRenderSystem() override = default;

//     static constexpr const char* Name() { return
//     "BouncyBouncyParticleRenderSystem"; }

//     DIX_DISABLE_COPY(BouncyBouncyParticleRenderSystem)

//     // BouncyBouncyParticle management
//     void updateBouncyParticles(float deltaTime);
//     GameObject createBouncyParticleEmitter(glm::vec3 position, uint32_t
//     count);

//     // Dispatches the compute shader. Call outside a render pass.
//     void dispatchCompute(vk::CommandBuffer commandBuffer) override;

//     void renderGameObjects(FrameInfo& frameInfo,
//                            std::vector<GameObject>& gameObjects) override;

//    protected:
//     // Writes m_particleBuffer and m_simulationParamsBuffer into
//     // the compute descriptor set allocated by the base class.
//     void buildComputeDescriptors() override;

//    private:
//     void bindBuffers(vk::CommandBuffer commandBuffer) const;

//     // BouncyBouncyParticle storage buffer: [uint32_t count |
//     // BouncyBouncyParticle[MAX_PARTICLES]]
//     std::unique_ptr<DixBuffer> m_particleBuffer;
//     // UBO for simulation params fed to the compute shader
//     std::unique_ptr<DixBuffer> m_simulationParamsBuffer;

//     BouncyParticleSimulationParams m_simParams{};
//     uint32_t m_particleCount{0};
//     static constexpr uint32_t MAX_PARTICLES = 1'000'000;
// };

// }  // namespace dix
// #endif  // BOUNCY_PARTICLE_RENDER_SYSTEM_HPP

#ifndef BOUNCY_PARTICLE_RENDER_SYSTEM_HPP
#define BOUNCY_PARTICLE_RENDER_SYSTEM_HPP

// dix
#include <Model/GameObject/ParticleEmitter.hpp>
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

struct BouncyParticleUbo {
    alignas(16) glm::mat4 projectionView{1.f};
};

struct BouncyParticlePushConstantData {
    alignas(16) glm::mat4 modelMatrix{1.f};
};

using BouncyParticleRenderSystemBindings = std::tuple<
    UniformBinding<BouncyParticleUbo, 0, vk::ShaderStageFlagBits::eVertex>>;

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
    void updateBouncyParticles(
        float deltaTime,
        const std::vector<std::shared_ptr<ParticleEmitter>>& particleEmitters);
    std::shared_ptr<ParticleEmitter> createBouncyParticleEmitter(
        glm::vec3 position, uint32_t count);

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
#endif  // BOUNCY_PARTICLE_RENDER_SYSTEM_HPP