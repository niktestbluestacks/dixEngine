// dix
#include <Pipeline/Buffer/DixBuffer.hpp>

 // std
#include <cassert>
#include <cstring>

namespace dix {
VkDeviceSize DixBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
    if (minOffsetAlignment > 0) {
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
}

DixBuffer::DixBuffer(
        EngineDevice& device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment)
        : m_dixDevice { device },
        instanceSize{ instanceSize },
        instanceCount{ instanceCount },
        usageFlags{ usageFlags },
        memoryPropertyFlags{ memoryPropertyFlags } {
    alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
    bufferSize = alignmentSize * instanceCount;
    device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, m_buffer, m_memory);
}

DixBuffer::~DixBuffer() {
    unmap();
    vkDestroyBuffer(m_dixDevice.device(), m_buffer, nullptr);
    vkFreeMemory(m_dixDevice.device(), m_memory, nullptr);
}

VkResult DixBuffer::map(VkDeviceSize size, VkDeviceSize offset) {
    assert(m_buffer && m_memory && "Called map on buffer before create");
    return vkMapMemory(m_dixDevice.device(), m_memory, offset, size, 0, &m_mapped);
}

/**
    * Unmap a mapped memory range
    *
    * @note Does not return a result as vkUnmapMemory can't fail
    */
void DixBuffer::unmap() {
    if (m_mapped) {
        vkUnmapMemory(m_dixDevice.device(), m_memory);
        m_mapped = nullptr;
    }
}

void DixBuffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset) {
    assert(m_mapped && "Cannot copy to unmapped buffer");

    if (size == VK_WHOLE_SIZE) {
        memcpy(m_mapped, data, bufferSize);
    }
    else {
        char* memOffset = static_cast <char*> (m_mapped);
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

VkResult DixBuffer::flush(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(m_dixDevice.device(), 1, &mappedRange);
}

VkResult DixBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(m_dixDevice.device(), 1, &mappedRange);
}

VkDescriptorBufferInfo DixBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
    return VkDescriptorBufferInfo{
        m_buffer,
        offset,
        size,
    };
}

void DixBuffer::writeToIndex(void* data, int index) {
    writeToBuffer(data, instanceSize, index * alignmentSize);
}

VkResult DixBuffer::flushIndex(int index) { return flush(alignmentSize, index * alignmentSize); }

VkDescriptorBufferInfo DixBuffer::descriptorInfoForIndex(int index) {
    return descriptorInfo(alignmentSize, index * alignmentSize);
}

VkResult DixBuffer::invalidateIndex(int index) {
    return invalidate(alignmentSize, index * alignmentSize);
}

}  // namespace dix