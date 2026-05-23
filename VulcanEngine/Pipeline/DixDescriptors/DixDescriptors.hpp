#ifndef _DIX_DESCRIPTOR_HPP_
#define _DIX_DESCRIPTOR_HPP_

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>

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
            VkDescriptorType descriptorType,
            VkShaderStageFlags stageFlags,
            uint32_t count = 1);
        std::unique_ptr<DixDescriptorSetLayout> build() const;

    private:
        EngineDevice& engineDevice;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
    };

    DixDescriptorSetLayout(
        EngineDevice& engineDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
    ~DixDescriptorSetLayout();
    DixDescriptorSetLayout(const DixDescriptorSetLayout&) = delete;
    DixDescriptorSetLayout& operator=(const DixDescriptorSetLayout&) = delete;

    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& getBindings() const {
        return bindings;
    }

private:
    EngineDevice& engineDevice;
    VkDescriptorSetLayout descriptorSetLayout;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

    friend class DixDescriptorWriter;
};

class DixDescriptorPool {
public:
    class Builder {
    public:
        Builder(EngineDevice& engineDevice) : engineDevice{ engineDevice } {}

        Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
        Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
        Builder& setMaxSets(uint32_t count);
        std::unique_ptr<DixDescriptorPool> build() const;

    private:
        EngineDevice& engineDevice;
        std::vector<VkDescriptorPoolSize> poolSizes{};
        uint32_t maxSets = 1000;
        VkDescriptorPoolCreateFlags poolFlags = 0;
    };

    DixDescriptorPool(
        EngineDevice& engineDevice,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes);
    ~DixDescriptorPool();
    DixDescriptorPool(const DixDescriptorPool&) = delete;
    DixDescriptorPool& operator=(const DixDescriptorPool&) = delete;

    bool allocateDescriptorSet(
        const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

    void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

    void resetPool();

private:
    EngineDevice& engineDevice;
    VkDescriptorPool descriptorPool;

    friend class DixDescriptorWriter;
};

class DixDescriptorWriter {
public:
    DixDescriptorWriter(DixDescriptorSetLayout& setLayout, DixDescriptorPool& pool);

    DixDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
    DixDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

    DixDescriptorWriter& writeImageSampler(
        uint32_t binding,
        VkImageView imageView,
        VkSampler sampler,
        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    bool build(VkDescriptorSet& set);
    void overwrite(VkDescriptorSet& set);

private:
    DixDescriptorSetLayout& setLayout;
    DixDescriptorPool& pool;
    std::vector<VkWriteDescriptorSet> writes;
    std::vector<VkDescriptorImageInfo> imageInfos;
    std::vector<VkDescriptorBufferInfo> bufferInfos;
};

}  // namespace dix
#endif // _DIX_DESCRIPTOR_HPP_