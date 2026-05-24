// dix
#include <Rendering/RenderSystem/BoucyParticleRenderSystem/BouncyParticleRenderSystem.hpp>
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>
#include <Utils/Converter.hpp>

// std
#include <cassert>
#include <random>
#include <stdexcept>

namespace dix {

BouncyParticleRenderSystem::BouncyParticleRenderSystem(
    EngineDevice& engineDevice,
    VkRenderPass renderPass,
    VkDescriptorSetLayout globalSetLayout,
    VkDescriptorSetLayout modelSetLayout)
    : DixRenderSystem(
        engineDevice,
        renderPass,
        globalSetLayout,
        modelSetLayout,
        DixRenderSystemConfig{
            .vertShaderPath = "BouncyParticleShader/bouncy_particle.vert.spv",
            .fragShaderPath = "BouncyParticleShader/bouncy_particle.frag.spv",

            // Point-list: each BouncyParticle is a screen-space point / sprite.
            .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,

            // Vertex layout mirrors the BouncyParticle struct.
            .vertexBindings = {
                { 0, sizeof(BouncyParticle), VK_VERTEX_INPUT_RATE_VERTEX }
            },
            .vertexAttributes = {
                { 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT,    offsetof(BouncyParticle, positionLifetime) },
                { 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT,    offsetof(BouncyParticle, velocitySize) },     
                { 2, 0, VK_FORMAT_R32G32B32A32_SFLOAT,    offsetof(BouncyParticle, color) },
                { 3, 0, VK_FORMAT_R32G32B32A32_SFLOAT,    offsetof(BouncyParticle, initPosLife)     },
            },

            .transformGameObject = [](void* push, GameObject& obj, FrameInfo& frameInfo) {
                auto* p       = static_cast<BouncyParticlePushConstantData*>(push);
                p->modelMatrix = obj.transform.mat4();
            },
            .pushConstantSize = sizeof(BouncyParticlePushConstantData),

            // Compute pipeline: the base constructor wires up the descriptor-set
            // layout, pipeline layout, ComputePipeline, and descriptor pool from
            // these entries.  buildComputeDescriptors() is called below once
            // the data buffers have been created.
            .compute = ComputePipelineConfig{
                .shaderPath = "BouncyParticleShader/bouncy_particle.comp.spv",
                .bindings   = {
                    // binding 0: BouncyParticle SSBO (read/write by vertex + compute)
                    { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                      VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT },
                    // binding 1: simulation params UBO (compute only)
                    { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                      VK_SHADER_STAGE_COMPUTE_BIT },
                },
            },
        }) {
    // 1. BouncyParticle storage buffer  [uint32_t count | BouncyParticle × MAX]
    VkDeviceSize particleBufferSize = 16 + sizeof(BouncyParticle) * MAX_PARTICLES;
    m_particleBuffer = std::make_unique<DixBuffer>(
        engineDevice,
        particleBufferSize,
        1,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    m_particleBuffer->map();
    uint32_t zeroCount = 0;
    m_particleBuffer->writeToBuffer(&zeroCount, sizeof(uint32_t));
    m_particleBuffer->unmap();

    // 2. Simulation params UBO
    m_simulationParamsBuffer = std::make_unique<DixBuffer>(
        engineDevice,
        sizeof(BouncyParticleSimulationParams),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    m_simParams.gravityDeltaTime.w = 0.016f;
    m_simulationParamsBuffer->map();
    m_simulationParamsBuffer->writeToBuffer(&m_simParams, sizeof(BouncyParticleSimulationParams));
    m_simulationParamsBuffer->unmap();

    // 3. Write the compute descriptor set now that both buffers exist.
    //    The base constructor already created the layout, pipeline, and pool.
    buildComputeDescriptors();
}

void BouncyParticleRenderSystem::buildComputeDescriptors() {
    assert(m_particleBuffer         && "BouncyParticle buffer must be created first");
    assert(m_simulationParamsBuffer && "Sim-params buffer must be created first");
    assert(m_computeSetLayout       && "Compute set layout not initialised");
    assert(m_computeDescriptorPool  && "Compute descriptor pool not initialised");

    // BouncyParticle SSBO: skip the leading uint32_t BouncyParticle-count header.
    VkDescriptorBufferInfo particleInfo{};
    particleInfo.buffer = m_particleBuffer->getBuffer();
    particleInfo.offset = 0;
    particleInfo.range =
    16 + sizeof(BouncyParticle) * MAX_PARTICLES;

    VkDescriptorBufferInfo simParamsInfo = m_simulationParamsBuffer->descriptorInfo();

    bool ok = DixDescriptorWriter(*m_computeSetLayout, *m_computeDescriptorPool)
        .writeBuffer(0, &particleInfo)
        .writeBuffer(1, &simParamsInfo)
        .build(m_computeDescriptorSet);

    if (!ok) {
        throw std::runtime_error("BouncyParticleRenderSystem: failed to build compute descriptor set");
    }
}

// dispatchCompute  (overrides base no-op)
//
// Dispatches the BouncyParticle simulation compute shader, then
// inserts a compute → vertex/shader memory barrier so the
// GPU sees the updated SSBO contents during the subsequent
// graphics pass.  Must be called OUTSIDE a render pass.

void BouncyParticleRenderSystem::dispatchCompute(VkCommandBuffer commandBuffer) {
    if (m_particleCount == 0 || !hasComputePipeline()) return;

    m_computePipeline->bind(commandBuffer);

    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        m_computePipelineLayout,
        0, 1, &m_computeDescriptorSet,
        0, nullptr);

    // local_size_x = 64 in the compute shader
    uint32_t workGroups = (m_particleCount + 63) / 64;
    vkCmdDispatch(commandBuffer, workGroups, 1, 1);

    // Barrier: wait for the compute shader's SSBO writes to be visible
    // to the vertex input stage and the vertex shader (which reads the
    // BouncyParticle SSBO bound at set 1, binding 0).
    VkMemoryBarrier barrier{};
    barrier.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT
                          | VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        0,
        1, &barrier,
        0, nullptr,
        0, nullptr);
}

void BouncyParticleRenderSystem::renderGameObjects(
        FrameInfo& frameInfo,
        std::vector<GameObject>& gameObjects) {
    m_pipeline->bind(frameInfo.commandBuffer);
    bindBuffers(frameInfo.commandBuffer);

    VkViewport viewport{
        0.0f, 0.0f,
        static_cast<float>(frameInfo.screenExtent.width),
        static_cast<float>(frameInfo.screenExtent.height),
        0.0f, 1.0f
    };
    vkCmdSetViewport(frameInfo.commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{ {0, 0}, frameInfo.screenExtent };
    vkCmdSetScissor(frameInfo.commandBuffer, 0, 1, &scissor);

    updateParticles(frameInfo.frameTime);

    for (auto& obj : gameObjects) {
        std::array<std::byte, MAX_PUSH_CONSTANT_BYTES> pushBuffer{};
        m_config.transformGameObject(pushBuffer.data(), obj, frameInfo);

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            m_pipelineLayout,
            m_config.pushConstantStages,
            0,
            m_config.pushConstantSize,
            pushBuffer.data());

        // set 0: global UBO (camera / projection)
        // set 1: BouncyParticle compute set (SSBO + sim-params)
        std::array<VkDescriptorSet, 2> descriptorSets{
            frameInfo.globalDescriptorSet,
            m_computeDescriptorSet
        };

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            static_cast<uint32_t>(descriptorSets.size()),
            descriptorSets.data(),
            0, nullptr);

        if (m_particleCount > 0) {
            vkCmdDraw(frameInfo.commandBuffer, m_particleCount, 1, 0, 0);
        }
    }
}

void BouncyParticleRenderSystem::bindBuffers(VkCommandBuffer commandBuffer) const {
    // VkBuffer buffers[] = { m_particleBuffer->getBuffer() };
    // VkDeviceSize offsets[] = { sizeof(uint32_t) }; // skip the count header
    // vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    VkBuffer buffers[] = { m_particleBuffer->getBuffer() };

    // std430 alignment:
    // uint particleCount -> 4 bytes
    // next struct array aligned to 16 bytes
    VkDeviceSize offsets[] = { 16 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void BouncyParticleRenderSystem::updateParticles(float deltaTime) {
    m_simParams.gravityDeltaTime.w = deltaTime;
    m_simulationParamsBuffer->map();
    m_simulationParamsBuffer->writeToBuffer(&m_simParams, sizeof(BouncyParticleSimulationParams));
    m_simulationParamsBuffer->unmap();
}

void BouncyParticleRenderSystem::createParticleEmitter(glm::vec3 position, uint32_t count) {
    if (m_particleCount + count > MAX_PARTICLES) {
        count = MAX_PARTICLES - m_particleCount;
    }
    if (count == 0) return;

    m_simParams.particlesPosLife = glm::vec4(position, m_simParams.particlesPosLife.w);

    std::mt19937 gen{ std::random_device{}() };
    std::uniform_real_distribution<float> posDist(-0.5f,  0.5f);
    std::uniform_real_distribution<float> velDist(-2.0f,  2.0f);
    std::uniform_real_distribution<float> colDist( 0.5f,  1.0f);

    m_particleBuffer->map();

    auto* countPtr = static_cast<uint32_t*>(m_particleBuffer->getMappedMemory());
    *countPtr = m_particleCount + count;

    auto* particles = reinterpret_cast<BouncyParticle*>(
        static_cast<uint8_t*>(m_particleBuffer->getMappedMemory()) + 16);

    for (uint32_t i = 0; i < count; ++i) {
        BouncyParticle& p = particles[m_particleCount + i];
        p.positionLifetime = glm::vec4(position + glm::vec3(posDist(gen), posDist(gen), posDist(gen)),
        m_simParams.particlesPosLife.w);
        p.initVelocity = glm::vec3(velDist(gen), velDist(gen) * 0.5f, velDist(gen));
        p.velocitySize = glm::vec4(p.initVelocity, 3.f);
        p.color = glm::vec4(colDist(gen), colDist(gen), colDist(gen), 1.0f);
        p.initPosLife = p.positionLifetime;
    }
    m_particleBuffer->unmap();
    m_particleCount += count;
}

}   // namespace dix
