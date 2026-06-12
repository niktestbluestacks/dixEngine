#ifndef PARTICLE_EMITTER_HPP
#define PARTICLE_EMITTER_HPP

#include <Model/GameObject/GameObject.hpp>

namespace dix {
struct ParticleSimulationParams {
    alignas(16) glm::vec4 gravityDeltaTime{0.f, -.1f, 0.f, 0.f};
    alignas(16) glm::vec4 windDamping{0.f, 0.f, 0.f, .999f};
    alignas(16) glm::vec4 particlesPosLife{0.f, 0.f, 0.f, 1000.f};
};

class ParticleEmitter : public GameObject {
   public:
    DIX_ENABLE_MOVE(ParticleEmitter)
    using GameObject::GameObject;
    ParticleEmitter()
        : GameObject{},
          particleBuffer{nullptr},
          simulationParamsBuffer{nullptr},
          simParams{},
          particleCount{} {}
    // Particle storage buffer: [uint32_t count | Particle[MAX_PARTICLES]]
    std::unique_ptr<DixBuffer> particleBuffer;
    // UBO for simulation params fed to the compute shader
    std::unique_ptr<DixBuffer> simulationParamsBuffer;

    ParticleSimulationParams simParams;
    uint32_t particleCount;
    vk::DescriptorSet computeDescriptorSet;
    static constexpr uint32_t MAX_PARTICLES = 10'000'000;
};
}  // namespace dix

#endif  // PARTICLE_EMITTER_HPP