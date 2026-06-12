// dix
#include <Pipeline/Buffer/DixBuffer.hpp>

// libs
#include <vulkan/vulkan.hpp>

// std
#include <cassert>
#include <cstring>

namespace dix {
vk::DeviceSize DixBuffer::getAlignment(vk::DeviceSize instanceSize,
                                       vk::DeviceSize minOffsetAlignment) {
    if (minOffsetAlignment > 0) {
        return (instanceSize + minOffsetAlignment - 1) &
               ~(minOffsetAlignment - 1);
    }
    return instanceSize;
}

DixBuffer::DixBuffer(EngineDevice& device, vk::DeviceSize instanceSize,
                     uint32_t instanceCount, vk::BufferUsageFlags usageFlags,
                     vk::MemoryPropertyFlags memoryPropertyFlags,
                     vk::DeviceSize minOffsetAlignment)
    : m_dixDevice{device},
      instanceSize{instanceSize},
      instanceCount{instanceCount},
      usageFlags{usageFlags},
      memoryPropertyFlags{memoryPropertyFlags} {
    alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
    bufferSize = alignmentSize * instanceCount;
    device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, m_buffer,
                        m_memory);
}

DixBuffer::~DixBuffer() {
    if (m_mapped && m_buffer) {
        m_dixDevice.device().unmapMemory(m_memory);
        m_mapped = nullptr;
        m_dixDevice.device().destroyBuffer(m_buffer);
        m_dixDevice.device().freeMemory(m_memory);
    }
}

vk::Result DixBuffer::map(vk::DeviceSize size, vk::DeviceSize offset) {
    assert(m_buffer && m_memory && "Called map on buffer before create");
    return m_dixDevice.device().mapMemory(m_memory, offset, size, {},
                                          &m_mapped);
}

/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as vk::UnmapMemory can't fail
 */
void DixBuffer::unmap() {
    if (m_mapped) {
        m_dixDevice.device().unmapMemory(m_memory);
        m_mapped = nullptr;
    }
}

void DixBuffer::writeToBuffer(void* data, vk::DeviceSize size,
                              vk::DeviceSize offset) {
    assert(m_mapped && "Cannot copy to unmapped buffer");

    if (size == VK_WHOLE_SIZE) {
        memcpy(m_mapped, data, bufferSize);
    } else {
        char* memOffset = static_cast<char*>(m_mapped);
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

vk::Result DixBuffer::flush(vk::DeviceSize size, vk::DeviceSize offset) {
    vk::MappedMemoryRange mappedRange{};
    mappedRange.memory = m_memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    m_dixDevice.device().flushMappedMemoryRanges({mappedRange});
    return vk::Result::eSuccess;
}

vk::Result DixBuffer::invalidate(vk::DeviceSize size, vk::DeviceSize offset) {
    vk::MappedMemoryRange mappedRange{};
    mappedRange.memory = m_memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    m_dixDevice.device().invalidateMappedMemoryRanges({mappedRange});
    return vk::Result::eSuccess;
}

vk::DescriptorBufferInfo DixBuffer::descriptorInfo(vk::DeviceSize size,
                                                   vk::DeviceSize offset) {
    return vk::DescriptorBufferInfo{
        m_buffer,
        offset,
        size,
    };
}

void DixBuffer::writeToIndex(void* data, int index) {
    writeToBuffer(data, instanceSize, index * alignmentSize);
}

vk::Result DixBuffer::flushIndex(int index) {
    return flush(alignmentSize, index * alignmentSize);
}

vk::DescriptorBufferInfo DixBuffer::descriptorInfoForIndex(int index) {
    return descriptorInfo(alignmentSize, index * alignmentSize);
}

vk::Result DixBuffer::invalidateIndex(int index) {
    return invalidate(alignmentSize, index * alignmentSize);
}

}  // namespace dix