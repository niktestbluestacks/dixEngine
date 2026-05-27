// dix
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>

// libs
#include <vulkan/vulkan.hpp>

// std
#include <cassert>
#include <stdexcept>

namespace dix {

DixDescriptorSetLayout::Builder& DixDescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    vk::DescriptorType descriptorType,
    vk::ShaderStageFlags stageFlags,
    uint32_t count) {

    assert(!bindings.contains(binding) && "Binding already in use");
    vk::DescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    bindings[binding] = layoutBinding;
    return *this;
}

std::unique_ptr<DixDescriptorSetLayout> DixDescriptorSetLayout::Builder::build() const {
    return std::make_unique<DixDescriptorSetLayout>(engineDevice, bindings);
}


DixDescriptorSetLayout::DixDescriptorSetLayout(
    EngineDevice& engineDevice, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings)
    : engineDevice{ engineDevice }, bindings{ bindings } {
    std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings{};
    for (const auto& kv : bindings) {
        setLayoutBindings.push_back(kv.second);
    }

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

    auto result = engineDevice.device().createDescriptorSetLayout(descriptorSetLayoutInfo);
    // if (result != vk::Result::eSuccess) {
    //     throw std::runtime_error("failed to create descriptor set layout!");
    // }
    descriptorSetLayout = result;
}

DixDescriptorSetLayout::~DixDescriptorSetLayout() {
    engineDevice.device().destroyDescriptorSetLayout(descriptorSetLayout);
}

DixDescriptorPool::Builder& DixDescriptorPool::Builder::addPoolSize(
    vk::DescriptorType descriptorType, uint32_t count) {
    poolSizes.push_back({ descriptorType, count });
    return *this;
}

DixDescriptorPool::Builder& DixDescriptorPool::Builder::setPoolFlags(
    vk::DescriptorPoolCreateFlags flags) {
    poolFlags = flags;
    return *this;
}
DixDescriptorPool::Builder& DixDescriptorPool::Builder::setMaxSets(uint32_t count) {
    maxSets = count;
    return *this;
}

std::unique_ptr<DixDescriptorPool> DixDescriptorPool::Builder::build() const {
    return std::make_unique<DixDescriptorPool>(engineDevice, maxSets, poolFlags, poolSizes);
}

DixDescriptorPool::DixDescriptorPool(
    EngineDevice& engineDevice,
    uint32_t maxSets,
    vk::DescriptorPoolCreateFlags poolFlags,
    const std::vector<vk::DescriptorPoolSize>& poolSizes)
    : engineDevice{ engineDevice } {
    vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    auto result = engineDevice.device().createDescriptorPool(descriptorPoolInfo);
    // if (result != vk::Result::eSuccess) {
    //     throw std::runtime_error("failed to create descriptor pool!");
    // }
    descriptorPool = result;
}

DixDescriptorPool::~DixDescriptorPool() {
    if (descriptorPool) {
        engineDevice.device().destroyDescriptorPool(descriptorPool);
    }
}

bool DixDescriptorPool::allocateDescriptorSet(
    const vk::DescriptorSetLayout descriptorSetLayout, vk::DescriptorSet& descriptor) const {
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope
    auto result = engineDevice.device().allocateDescriptorSets(allocInfo);
    // if (result != vk::Result::eSuccess) {
    //     return false;
    // }
    descriptor = result.front();
    return true;
}

void DixDescriptorPool::freeDescriptors(std::vector<vk::DescriptorSet>& descriptors) const {
    engineDevice.device().freeDescriptorSets(descriptorPool, descriptors);
}

void DixDescriptorPool::resetPool() {
    engineDevice.device().resetDescriptorPool(descriptorPool);
}

DixDescriptorWriter::DixDescriptorWriter(DixDescriptorSetLayout& setLayout, DixDescriptorPool& pool)
    : setLayout{ setLayout }, pool{ pool } {
    bufferInfos.reserve(sizeof(vk::DescriptorBufferInfo) * 16);
    imageInfos.reserve(sizeof(vk::DescriptorImageInfo) * 16);
}

DixDescriptorWriter& DixDescriptorWriter::writeBuffer(
    uint32_t binding, vk::DescriptorBufferInfo* bufferInfo) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto& bindingDescription = setLayout.bindings[binding];

    assert(
        bindingDescription.descriptorCount == 1 &&
        "Binding single descriptor info, but binding expects multiple"
    );

    // store a copy owned by the writer and point the write entry to that storage
    bufferInfos.push_back(*bufferInfo);

    vk::WriteDescriptorSet write{};
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = &bufferInfos.back();
    write.descriptorCount = 1;

    writes.push_back(write);
    return *this;
}

DixDescriptorWriter& DixDescriptorWriter::writeImage(
    uint32_t binding, vk::DescriptorImageInfo* imageInfo) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto& bindingDescription = setLayout.bindings[binding];

    assert(
        bindingDescription.descriptorCount == 1 &&
        "Binding single descriptor info, but binding expects multiple"
    );

    // store a copy owned by the writer and point the write entry to that storage
    imageInfos.push_back(*imageInfo);

    vk::WriteDescriptorSet write{};
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = &imageInfos.back();
    write.descriptorCount = 1;

    writes.push_back(write);
    return *this;
}

DixDescriptorWriter& DixDescriptorWriter::writeImageSampler(
    uint32_t binding,
    vk::ImageView imageView,
    vk::Sampler sampler,
    vk::ImageLayout imageLayout) {
    vk::DescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = imageLayout;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;
    return writeImage(binding, &imageInfo);
}

bool DixDescriptorWriter::build(vk::DescriptorSet& set) {
    bool success = pool.allocateDescriptorSet(setLayout.getDescriptorSetLayout(), set);
    if (!success) {
        return false;
    }
    overwrite(set);
    return true;
}

void DixDescriptorWriter::overwrite(vk::DescriptorSet& set) {
    for (auto& write : writes) {
        write.dstSet = set;
    }
    pool.engineDevice.device().updateDescriptorSets(writes, {});
}

}  // namespace dix