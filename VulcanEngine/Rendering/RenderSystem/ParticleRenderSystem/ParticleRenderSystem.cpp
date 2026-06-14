// dix
#include <Logger/Console.hpp>
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>
#include <Rendering/RenderSystem/ParticleRenderSystem/ParticleRenderSystem.hpp>
#include <Utils/Converter.hpp>

// std
#include <cassert>
#include <random>
#include <stdexcept>
#include <thread>

namespace dix {

ParticleRenderSystem::ParticleRenderSystem(
    EngineDevice& engineDevice, vk::RenderPass renderPass,
    vk::DescriptorSetLayout globalSetLayout,
    vk::DescriptorSetLayout modelSetLayout)
    : DixRenderSystem(
          engineDevice, renderPass, globalSetLayout, modelSetLayout,
          DixRenderSystemConfig{
              .vertShaderPath = "ParticleShader/particle.vert.spv",
              .fragShaderPath = "ParticleShader/particle.frag.spv",

              // Point-list: each particle is a screen-space point / sprite.
              .topology = vk::PrimitiveTopology::ePointList,

              // Vertex layout mirrors the Particle struct.
              .vertexBindings = {{0, sizeof(Particle),
                                  vk::VertexInputRate::eVertex}},
              .vertexAttributes =
                  {
                      {0, 0, vk::Format::eR32G32B32A32Sfloat,
                       offsetof(Particle, positionLifetime)},
                      {1, 0, vk::Format::eR32G32B32A32Sfloat,
                       offsetof(Particle, velocitySize)},
                      {2, 0, vk::Format::eR32G32B32A32Sfloat,
                       offsetof(Particle, color)},
                      {3, 0, vk::Format::eR32G32B32A32Sfloat,
                       offsetof(Particle, initPosLife)},
                  },

              .transformGameObject =
                  [](void* push, GameObject& obj, FrameInfo& frameInfo) {
                      auto* p = static_cast<ParticlePushConstantData*>(push);
                      p->modelMatrix = obj.transform.mat4();
                  },
              .pushConstantSize = sizeof(ParticlePushConstantData),

              // Compute pipeline: the base constructor wires up the
              // descriptor-set layout, pipeline layout, ComputePipeline, and
              // descriptor pool from these entries.  buildComputeDescriptors()
              // is called below once the data buffers have been created.
              .compute =
                  ComputePipelineConfig{
                      .shaderPath = "ParticleShader/particle_compute.comp.spv",
                      .bindings =
                          {
                              // binding 0: particle SSBO (read/write by vertex
                              // + compute)
                              {0, vk::DescriptorType::eStorageBuffer,
                               vk::ShaderStageFlagBits::eCompute |
                                   vk::ShaderStageFlagBits::eVertex},
                              // binding 1: simulation params UBO (compute only)
                              {1, vk::DescriptorType::eUniformBuffer,
                               vk::ShaderStageFlagBits::eCompute},
                          },
                      .descriptorPoolMaxSets =
                          ParticleRenderSystem::MAX_PARTICLE_EMITTERS},
          }) {}

void ParticleRenderSystem::buildComputeDescriptors(ParticleEmitter& obj) {
    assert(obj.particleBuffer && "Particle buffer must be created first");
    assert(obj.simulationParamsBuffer &&
           "Sim-params buffer must be created first");
    assert(m_computeSetLayout && "Compute set layout not initialised");
    assert(m_computeDescriptorPool &&
           "Compute descriptor pool not initialised");

    // Particle SSBO: skip the leading uint32_t particle-count header.
    vk::DescriptorBufferInfo particleInfo{};
    particleInfo.buffer = obj.particleBuffer->getBuffer();
    particleInfo.offset = 0;
    particleInfo.range = 16 + sizeof(Particle) * obj.particleCount;

    vk::DescriptorBufferInfo simParamsInfo =
        obj.simulationParamsBuffer->descriptorInfo(
            sizeof(ParticleSimulationParams), 0);

    bool ok = DixDescriptorWriter(*m_computeSetLayout, *m_computeDescriptorPool)
                  .writeBuffer(0, &particleInfo)
                  .writeBuffer(1, &simParamsInfo)
                  .build(obj.computeDescriptorSet);

    if (!ok) {
        throw std::runtime_error(
            "ParticleRenderSystem: failed to build compute descriptor set");
    }
}

// Dispatches the particle simulation compute shader, then
// inserts a compute -> vertex/shader memory barrier so the
// GPU sees the updated SSBO contents during the subsequent
// graphics pass.  Must be called OUTSIDE a render pass.

void ParticleRenderSystem::dispatchCompute(
    vk::CommandBuffer commandBuffer,
    std::vector<std::shared_ptr<GameObject>>& gameObjects) {
    for (auto& unObject : gameObjects) {
        decltype(auto) obj =
            std::static_pointer_cast<ParticleEmitter>(unObject);

        m_computePipeline->bind(commandBuffer);

        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eCompute, m_computePipelineLayout, 0, 1,
            &obj->computeDescriptorSet, 0, nullptr);

        // local_size_x = 64 in the compute shader
        uint32_t workGroups = (obj->particleCount + 63) / 64;
        commandBuffer.dispatch(workGroups, 1, 1);

        // Barrier: wait for the compute shader's SSBO writes to be visible
        // to the vertex input stage and the vertex shader (which reads the
        // particle SSBO bound at set 1, binding 0).
        vk::MemoryBarrier barrier{};
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eVertexAttributeRead |
                                vk::AccessFlagBits::eShaderRead;

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eVertexInput |
                vk::PipelineStageFlagBits::eVertexShader,
            {}, barrier, {}, {});
    }
}

void ParticleRenderSystem::renderGameObjects(
    FrameInfo& frameInfo,
    std::vector<std::shared_ptr<GameObject>>& gameObjects) {
    m_pipeline->bind(frameInfo.commandBuffer);
    std::vector<std::shared_ptr<ParticleEmitter>> particleEmitters{};
    particleEmitters.reserve(gameObjects.size());
    std::transform(
        gameObjects.begin(), gameObjects.end(),
        std::back_inserter(particleEmitters),
        [](const std::shared_ptr<GameObject>& base_ptr) {
            return std::static_pointer_cast<ParticleEmitter>(base_ptr);
        });

    vk::Viewport viewport{0.0f,
                          0.0f,
                          static_cast<float>(frameInfo.screenExtent.width),
                          static_cast<float>(frameInfo.screenExtent.height),
                          0.0f,
                          1.0f};
    frameInfo.commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor{{}, frameInfo.screenExtent};
    frameInfo.commandBuffer.setScissor(0, 1, &scissor);

    updateParticles(frameInfo.frameTime, particleEmitters);

    for (auto& obj : particleEmitters) {
        std::array<std::byte, MAX_PUSH_CONSTANT_BYTES> pushBuffer{};
        bindBuffers(frameInfo.commandBuffer, obj);
        m_config.transformGameObject(pushBuffer.data(), *obj, frameInfo);

        frameInfo.commandBuffer.pushConstants(
            m_pipelineLayout, m_config.pushConstantStages, 0,
            m_config.pushConstantSize, pushBuffer.data());

        // set 0: system descriptor set — BouncyParticleUbo (projectionView
        // matrix).
        //        Written by AppContext each frame from m_systemUboBuffers.
        //
        // m_computeDescriptorSet is the compute-pipeline set (particle SSBO +
        // sim-params UBO).  It is bound exclusively during dispatchCompute() on
        // the COMPUTE bind point and must NOT be bound here: it was allocated
        // with m_computeSetLayout, which is incompatible with the model set
        // layout that the graphics pipeline layout expects at set 1.
        // The bouncy-particle vertex shader only accesses set 0, binding 0, so
        // set 1 does not need to be bound at all.
        std::array<vk::DescriptorSet, 1> descriptorSets{
            frameInfo.globalDescriptorSet};

        frameInfo.commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0,
            static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(),
            0, nullptr);

        if (obj->particleCount > 0) {
            frameInfo.commandBuffer.draw(obj->particleCount, 1, 0, 0);
        }
    }
}

void ParticleRenderSystem::bindBuffers(
    vk::CommandBuffer commandBuffer,
    std::shared_ptr<ParticleEmitter>& particleEmitter) const {
    // vk::Buffer buffers[] = { m_particleBuffer->getBuffer() };
    // vk::DeviceSize offsets[] = { sizeof(uint32_t) }; // skip the count header
    // commandBuffer.bindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    vk::Buffer buffers[] = {particleEmitter->particleBuffer->getBuffer()};
    vk::DeviceSize offsets[] = {16};

    // std430 alignment:
    // uint particleCount -> 4 bytes
    // next struct array aligned to 16 bytes
    commandBuffer.bindVertexBuffers(0, 1, buffers, offsets);
}

void ParticleRenderSystem::updateParticles(
    float deltaTime,
    const std::vector<std::shared_ptr<ParticleEmitter>>& particleEmitters) {
    for (auto& obj : particleEmitters) {
        obj->simParams.gravityDeltaTime.w = deltaTime;
        obj->simulationParamsBuffer->map();
        obj->simulationParamsBuffer->writeToBuffer(
            &obj->simParams, sizeof(ParticleSimulationParams));
        obj->simulationParamsBuffer->unmap();
    }
}

std::shared_ptr<ParticleEmitter> ParticleRenderSystem::createParticleEmitter(
    glm::vec3 position, uint32_t count, glm::vec2 colorDist) {
    std::shared_ptr<ParticleEmitter> obj = std::make_shared<ParticleEmitter>();
    // 1. Particle storage buffer  [uint32_t count | Particle × MAX]
    vk::DeviceSize particleBufferSize = 16 + sizeof(Particle) * count;
    obj->particleBuffer =
        std::make_unique<DixBuffer>(m_dixDevice, particleBufferSize, 1,
                                    vk::BufferUsageFlagBits::eStorageBuffer |
                                        vk::BufferUsageFlagBits::eVertexBuffer |
                                        vk::BufferUsageFlagBits::eTransferDst,
                                    vk::MemoryPropertyFlagBits::eDeviceLocal);

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    m_dixDevice.createBuffer(particleBufferSize,
                             vk::BufferUsageFlagBits::eTransferSrc,
                             vk::MemoryPropertyFlagBits::eHostVisible |
                                 vk::MemoryPropertyFlagBits::eHostCoherent,
                             stagingBuffer, stagingBufferMemory);

    void* stagingData;
    auto res = m_dixDevice.device().mapMemory(
        stagingBufferMemory, 0, particleBufferSize, {}, &stagingData);
    if (res != vk::Result::eSuccess) {
        auto& console = DixConsole::getDixConsole();
        console.logError("Failed to create particle emitter!");
        m_dixDevice.device().unmapMemory(stagingBufferMemory);
        return nullptr;
    }
    uint32_t zeroCount = 0;
    *static_cast<uint32_t*>(stagingData) = 0;

    obj->transform.translation = position;
    if (obj->particleCount + count > ParticleEmitter::MAX_PARTICLES) {
        count = std::min(ParticleEmitter::MAX_PARTICLES, count);
    }

    obj->simParams.particlesPosLife =
        glm::vec4(position, obj->simParams.particlesPosLife.w);

    // obj->particleBuffer->map();

    auto* countPtr = static_cast<uint32_t*>(stagingData);
    *countPtr = count;

    auto* particles =
        reinterpret_cast<Particle*>(static_cast<uint8_t*>(stagingData) + 16);

    const uint32_t NUM_THREADS =
        std::max(4u, std::thread::hardware_concurrency());
    std::vector<std::jthread> threads;
    threads.reserve(NUM_THREADS);

    std::random_device rd;
    uint32_t baseSeed = rd();

    uint32_t chunk_size = count / NUM_THREADS;
    uint32_t remainder = count % NUM_THREADS;

    uint32_t current_start = 0;

    for (uint32_t t = 0; t < NUM_THREADS; ++t) {
        uint32_t current_count = chunk_size + (t < remainder ? 1 : 0);
        threads.emplace_back([t, current_start, current_count, &position,
                              particles, baseSeed, &obj, colorDist, this]() {
            std::mt19937 gen{baseSeed + t};
            std::uniform_real_distribution<float> posDist(-0.5f, 0.5f);
            std::uniform_real_distribution<float> velDist(-2.0f, 2.0f);
            std::uniform_real_distribution<float> colDist(colorDist.x,
                                                          colorDist.y);

            for (uint32_t i = 0; i < current_count; ++i) {
                uint32_t global_idx = current_start + i;
                Particle& p = particles[global_idx];
                p.positionLifetime = glm::vec4(
                    position + glm::normalize(glm::vec3(
                                   posDist(gen), posDist(gen), posDist(gen))),
                    obj->simParams.particlesPosLife.w);
                // MAKES A SHPERE
                p.initVelocity = glm::normalize(
                    glm::vec3{velDist(gen), velDist(gen), velDist(gen)});
                p.velocitySize = glm::vec4(p.initVelocity, 3.f);
                p.color =
                    glm::vec4(colDist(gen), colDist(gen), colDist(gen), 1.0f);
                p.initPosLife = p.positionLifetime;
            }
        });

        current_start += current_count;
    }
    threads.clear();

    m_dixDevice.device().unmapMemory(stagingBufferMemory);

    m_dixDevice.copyBuffer(stagingBuffer, obj->particleBuffer->getBuffer(),
                           particleBufferSize);
    m_dixDevice.device().destroyBuffer(stagingBuffer);
    m_dixDevice.device().freeMemory(stagingBufferMemory);

    obj->particleCount += count;

    // 2. Simulation params UBO
    obj->simulationParamsBuffer = std::make_unique<DixBuffer>(
        m_dixDevice, sizeof(ParticleSimulationParams), 1,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);
    obj->simParams.gravityDeltaTime.w = 0.016f;
    obj->simulationParamsBuffer->map();
    obj->simulationParamsBuffer->writeToBuffer(
        &obj->simParams, sizeof(ParticleSimulationParams));
    obj->simulationParamsBuffer->unmap();

    // 3. Write the compute descriptor set now that both buffers exist.
    //    The base constructor already created the layout, pipeline, and pool.
    buildComputeDescriptors(*obj);

    return obj;
}

}  // namespace dix