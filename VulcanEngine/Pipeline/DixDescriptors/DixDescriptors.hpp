#ifndef _DIX_DESCRIPTOR_HPP_
#define _DIX_DESCRIPTOR_HPP_

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>

// libs
#include <vulkan/vulkan.hpp>

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace dix {

class DixDescriptorSetLayout {
public:
    class Builder {
    public:
        Builder(EngineDevice& engineDevice) : engineDevice{ engineDevice } {}

        Builder& addBinding(
            uint32_t binding,
            vk::DescriptorType descriptorType,
            vk::ShaderStageFlags stageFlags,
            uint32_t count = 1);
        std::unique_ptr<DixDescriptorSetLayout> build() const;

    private:
        EngineDevice& engineDevice;
        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};
    };

    DixDescriptorSetLayout(
        EngineDevice& engineDevice, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings);
    ~DixDescriptorSetLayout();
    DixDescriptorSetLayout(const DixDescriptorSetLayout&) = delete;
    DixDescriptorSetLayout& operator=(const DixDescriptorSetLayout&) = delete;

    vk::DescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    const std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>& getBindings() const {
        return bindings;
    }

private:
    EngineDevice& engineDevice;
    vk::DescriptorSetLayout descriptorSetLayout;
    std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings;

    friend class DixDescriptorWriter;
};

class DixDescriptorPool {
public:
    class Builder {
    public:
        Builder(EngineDevice& engineDevice) : engineDevice{ engineDevice } {}

        Builder& addPoolSize(vk::DescriptorType descriptorType, uint32_t count);
        Builder& setPoolFlags(vk::DescriptorPoolCreateFlags flags);
        Builder& setMaxSets(uint32_t count);
        std::unique_ptr<DixDescriptorPool> build() const;

    private:
        EngineDevice& engineDevice;
        std::vector<vk::DescriptorPoolSize> poolSizes{};
        uint32_t maxSets = 1000;
        vk::DescriptorPoolCreateFlags poolFlags = {};
    };

    DixDescriptorPool(
        EngineDevice& engineDevice,
        uint32_t maxSets,
        vk::DescriptorPoolCreateFlags poolFlags,
        const std::vector<vk::DescriptorPoolSize>& poolSizes);
    ~DixDescriptorPool();
    DixDescriptorPool(const DixDescriptorPool&) = delete;
    DixDescriptorPool& operator=(const DixDescriptorPool&) = delete;

    bool allocateDescriptorSet(
        const vk::DescriptorSetLayout descriptorSetLayout, vk::DescriptorSet& descriptor
    ) const;

    void freeDescriptors(std::vector<vk::DescriptorSet>& descriptors) const;

    void resetPool();

private:
    EngineDevice& engineDevice;
    vk::DescriptorPool descriptorPool;

    friend class DixDescriptorWriter;
};

class DixDescriptorWriter {
public:
    DixDescriptorWriter(DixDescriptorSetLayout& setLayout, DixDescriptorPool& pool);

    DixDescriptorWriter& writeBuffer(uint32_t binding, vk::DescriptorBufferInfo* bufferInfo);
    DixDescriptorWriter& writeImage(uint32_t binding, vk::DescriptorImageInfo* imageInfo);

    DixDescriptorWriter& writeImageSampler(
        uint32_t binding,
        vk::ImageView imageView,
        vk::Sampler sampler,
        vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
    );

    bool build(vk::DescriptorSet& set);
    void overwrite(vk::DescriptorSet& set);

private:
    DixDescriptorSetLayout& setLayout;
    DixDescriptorPool& pool;
    std::vector<vk::WriteDescriptorSet> writes;
    std::vector<vk::DescriptorImageInfo> imageInfos;
    std::vector<vk::DescriptorBufferInfo> bufferInfos;
};

}  // namespace dix
#endif // _DIX_DESCRIPTOR_HPP_