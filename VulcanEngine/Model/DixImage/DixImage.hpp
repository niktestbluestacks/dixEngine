#ifndef DIX_IMAGE_HPP
#define DIX_IMAGE_HPP

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>

// libs
#include <vulkan/vulkan.hpp>

// std
#include <string>
#include <stdexcept>

namespace dix {
class DixImage {
public:
    DixImage();

    DixImage(
        EngineDevice& dixDevice,
        VkFormat format,
        uint32_t width,
        uint32_t height,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties
    );

    ~DixImage();

    VkImage& getImage() { return m_image; };
    VkDeviceMemory& getMemory() { return m_memory; };
    VkImageView& getImageView() { return m_view; };

    // Release ownership of the underlying Vulkan handles. After this call
    // the DixImage will no longer destroy the returned handles.
    std::tuple<VkImage, VkDeviceMemory, VkImageView> releaseOwnership();

private:
    void createImage(
        EngineDevice& device,
        VkFormat format,
        uint32_t width,
        uint32_t height,
        VkImageTiling tiling,
        VkImageUsageFlags usage
    );

    void allocateMemory(
        EngineDevice& device,
        VkMemoryPropertyFlags properties
    );

    void createImageView(EngineDevice& device, VkFormat format);

    VkImage m_image;
    VkDeviceMemory m_memory;
    VkImageView m_view;

    uint32_t m_width;
    uint32_t m_height;
    EngineDevice* m_dixDevice;
};
}

#endif // DIX_IMAGE_HPP