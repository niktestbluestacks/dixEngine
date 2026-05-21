#ifndef DIX_BUFFER_HPP
#define DIX_BUFFER_HPP

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>

namespace dix {
    
class DixBuffer {
public:
    DixBuffer(
        EngineDevice& device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment = 1);
    ~DixBuffer();

    DixBuffer(const DixBuffer&) = delete;
    DixBuffer& operator=(const DixBuffer&) = delete;

    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();

    void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    void writeToIndex(void* data, int index);
    VkResult flushIndex(int index);
    VkDescriptorBufferInfo descriptorInfoForIndex(int index);
    VkResult invalidateIndex(int index);

    VkBuffer getBuffer() const { return m_buffer; }
    void* getMappedMemory() const { return m_mapped; }
    uint32_t getInstanceCount() const { return instanceCount; }
    VkDeviceSize getInstanceSize() const { return instanceSize; }
    VkDeviceSize getAlignmentSize() const { return instanceSize; }
    VkBufferUsageFlags getUsageFlags() const { return usageFlags; }
    VkMemoryPropertyFlags getMemoryPropertyFlags() const { return memoryPropertyFlags; }
    VkDeviceSize getBufferSize() const { return bufferSize; }

private:
    static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

    EngineDevice& m_dixDevice;
    void* m_mapped = nullptr;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;

    VkDeviceSize bufferSize;
    uint32_t instanceCount;
    VkDeviceSize instanceSize;
    VkDeviceSize alignmentSize;
    VkBufferUsageFlags usageFlags;
    VkMemoryPropertyFlags memoryPropertyFlags;
};

}  // namespace dix


#endif // DIX_BUFFER_HPP
