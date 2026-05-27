#ifndef DIX_BUFFER_HPP
#define DIX_BUFFER_HPP

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>

// libs
#include <vulkan/vulkan.hpp>

namespace dix {

class DixBuffer {
public:
    DixBuffer(
        EngineDevice& device,
        vk::DeviceSize instanceSize,
        uint32_t instanceCount,
        vk::BufferUsageFlags usageFlags,
        vk::MemoryPropertyFlags memoryPropertyFlags,
        vk::DeviceSize minOffsetAlignment = 1);
    ~DixBuffer();

    DixBuffer(const DixBuffer&) = delete;
    DixBuffer& operator=(const DixBuffer&) = delete;

    vk::Result map(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    void unmap();

    void writeToBuffer(void* data, vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    vk::Result flush(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    vk::DescriptorBufferInfo descriptorInfo(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    vk::Result invalidate(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);

    void writeToIndex(void* data, int index);
    vk::Result flushIndex(int index);
    vk::DescriptorBufferInfo descriptorInfoForIndex(int index);
    vk::Result invalidateIndex(int index);

    vk::Buffer getBuffer() const { return m_buffer; }
    void* getMappedMemory() const { return m_mapped; }
    uint32_t getInstanceCount() const { return instanceCount; }
    vk::DeviceSize getInstanceSize() const { return instanceSize; }
    vk::DeviceSize getAlignmentSize() const { return instanceSize; }
    vk::BufferUsageFlags getUsageFlags() const { return usageFlags; }
    vk::MemoryPropertyFlags getMemoryPropertyFlags() const { return memoryPropertyFlags; }
    vk::DeviceSize getBufferSize() const { return bufferSize; }

private:
    static vk::DeviceSize getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment);

    EngineDevice& m_dixDevice;
    void* m_mapped = nullptr;
    vk::Buffer m_buffer;
    vk::DeviceMemory m_memory;

    vk::DeviceSize bufferSize;
    uint32_t instanceCount;
    vk::DeviceSize instanceSize;
    vk::DeviceSize alignmentSize;
    vk::BufferUsageFlags usageFlags;
    vk::MemoryPropertyFlags memoryPropertyFlags;
};

}  // namespace dix


#endif // DIX_BUFFER_HPP