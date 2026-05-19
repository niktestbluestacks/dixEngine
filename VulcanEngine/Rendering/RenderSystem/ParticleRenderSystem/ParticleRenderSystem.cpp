// dix
#include <Rendering/RenderSystem/ParticleRenderSystem/ParticleRenderSystem.hpp>
#include <Utils/Converter.hpp>
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>

// std
#include <random>

namespace dix {

ParticleRenderSystem::ParticleRenderSystem(
    EngineDevice& engineDevice,
    VkRenderPass renderPass,
    VkDescriptorSetLayout globalSetLayout,
    VkDescriptorSetLayout modelSetLayout) :
    DixRenderSystem(
        engineDevice,
        renderPass,
        globalSetLayout,
        modelSetLayout,
        "ParticleShader/particle.vert.spv",
        "ParticleShader/particle.frag.spv",
        [](void* pushConstantData, GameObject& obj) {
                auto* particlePush = static_cast<ParticlePushConstantData*>(pushConstantData);
                particlePush->modelMatrix = obj.transform.mat4();
        }
    ) {

        // Create particle storage buffer
        VkDeviceSize particleBufferSize = sizeof(uint32_t) + sizeof(Particle) * MAX_PARTICLES;
        m_particleBuffer = std::make_unique<DixBuffer>(
            engineDevice,
            particleBufferSize,
            1,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        // Create simulation params uniform buffer
        m_simulationParamsBuffer = std::make_unique<DixBuffer>(
            engineDevice,
            sizeof(ParticleSimulationParams),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        // Initialize simulation params
        m_simParams.deltaTime = 0.016f;
        m_simulationParamsBuffer->map();
        m_simulationParamsBuffer->writeToBuffer(&m_simParams, sizeof(ParticleSimulationParams));
        m_simulationParamsBuffer->unmap();

        createPipelineLayout(globalSetLayout, modelSetLayout);
        createComputePipeline(globalSetLayout, modelSetLayout);
        createPipeline(renderPass);
        setupDescriptors();
}

ParticleRenderSystem::~ParticleRenderSystem() {
    if (m_computePipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_dixDevice.device(), m_computePipelineLayout, nullptr);
    }
}

void ParticleRenderSystem::createComputePipeline(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout modelSetLayout) {
    // Create descriptor set layout for compute pipeline using DixDescriptorSetLayout
    // Binding 0: Storage buffer for particles (used by both compute and vertex shaders)
    // Binding 1: Uniform buffer for simulation params (used by compute shader)
    auto computeSetLayoutBuilder = DixDescriptorSetLayout::Builder(m_dixDevice);
    computeSetLayoutBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT);
    computeSetLayoutBuilder.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT);

    m_computeSetLayout = computeSetLayoutBuilder.build();

    // Create pipeline layout for compute
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = 0; // No push constants needed for compute in this example

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    VkDescriptorSetLayout setLayout = m_computeSetLayout->getDescriptorSetLayout();
    pipelineLayoutInfo.pSetLayouts = &setLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(m_dixDevice.device(), &pipelineLayoutInfo, nullptr, &m_computePipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout");
    }

    // Create compute pipeline using ComputePipelineConfigInfo
    ComputePipelineConfigInfo computeConfigInfo{};
    computeConfigInfo.pipelineLayout = m_computePipelineLayout;

    m_computePipeline = std::make_unique<ComputePipeline>(
        m_dixDevice,
        toShaderPath("ParticleShader/particle_compute.comp.spv"),
        computeConfigInfo
    );
}

void ParticleRenderSystem::setupDescriptors() {
    // Descriptor sets are created per-frame in AppContext via getVulkanFlags()
    // This method is a placeholder if additional setup is needed
}

void ParticleRenderSystem::dispatchCompute(VkCommandBuffer commandBuffer, uint32_t particleCount) {
    if (particleCount == 0 || !m_computePipeline) {
            return;
    }

    // Bind compute pipeline
    m_computePipeline->bind(commandBuffer);

    // Bind descriptor set for compute (storage buffer + simulation params)
    VkDescriptorSet computeDescriptorSet = VK_NULL_HANDLE;
    // Note: You need to create and store compute descriptor sets similar to graphics ones
    // For now, this assumes you'll integrate with the existing descriptor system

    // vkCmdBindDescriptorSets(
    //      commandBuffer,
    //      VK_PIPELINE_BIND_POINT_COMPUTE,
    //      m_computePipelineLayout,
    //      0,
    //      1,
    //      &computeDescriptorSet,
    //      0,
    //      nullptr
    // );

    // Dispatch compute shader
    uint32_t workGroups = (particleCount + 63) / 64; // Assuming local_size_x = 64 in compute shader
    vkCmdDispatch(commandBuffer, workGroups, 1, 1);
}

void ParticleRenderSystem::updateParticles(float deltaTime) {
    m_simParams.deltaTime = deltaTime;

    // Update simulation params buffer
    m_simulationParamsBuffer->map();
    m_simulationParamsBuffer->writeToBuffer(&m_simParams, sizeof(ParticleSimulationParams));
    m_simulationParamsBuffer->unmap();
}

void ParticleRenderSystem::createParticleEmitter(glm::vec3 position, uint32_t count) {
    if (m_particleCount + count > MAX_PARTICLES) {
        count = MAX_PARTICLES - m_particleCount;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-0.5f, 0.5f);
    std::uniform_real_distribution<float> velDist(-2.0f, 2.0f);
    std::uniform_real_distribution<float> colorDist(0.5f, 1.0f);

    // Map buffer and write particles
    m_particleBuffer->map();

    uint32_t* particleCountPtr = static_cast<uint32_t*>(m_particleBuffer->getMappedMemory());
    *particleCountPtr = m_particleCount + count;

    Particle* particles = reinterpret_cast<Particle*>(static_cast<uint8_t*>(m_particleBuffer->getMappedMemory()) + sizeof(uint32_t));

    for (uint32_t i = 0; i < count; ++i) {
        Particle& p = particles[m_particleCount + i];
        p.position = position + glm::vec3(posDist(gen), posDist(gen), posDist(gen));
        p.velocity = glm::vec3(velDist(gen), velDist(gen) * 0.5f, velDist(gen));
        p.lifetime = 1.0f;
        p.size = 0.1f;
        p.color = glm::vec4(colorDist(gen), colorDist(gen) * 0.5f, colorDist(gen) * 0.2f, 1.0f);
    }

    m_particleBuffer->unmap();
    m_particleCount += count;
}

}       // namespace dix