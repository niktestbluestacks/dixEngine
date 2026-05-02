#include "Pipeline/EngineDevice/EngineDevice.hpp"
#include <Model/DixImage/DixImage.hpp>

namespace dix {
static void checkVulkanResult(VkResult res, const char* msg) {
    if (res != VK_SUCCESS) {
        throw std::runtime_error(std::string(msg) + " (VK error " + std::to_string(res) + ")");
    }
}


DixImage::DixImage():
    m_image(VK_NULL_HANDLE),
    m_memory(VK_NULL_HANDLE),
    m_view(VK_NULL_HANDLE),
    m_height(0),
    m_width(0){}

DixImage::DixImage(
    EngineDevice& dixDevice,
    VkFormat format,
    uint32_t width,
    uint32_t height,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties
):      m_width(width), m_height(height), m_dixDevice(&dixDevice) {
    createImage(dixDevice, format, width, height, tiling, usage);

    allocateMemory(dixDevice, properties);

    createImageView(dixDevice, format);
}
 
DixImage::~DixImage() {
    if (m_view)        vkDestroyImageView(m_dixDevice->device(), m_view, nullptr);
    if (m_image)       vkDestroyImage(m_dixDevice->device(), m_image, nullptr);
    if (m_memory)      vkFreeMemory(m_dixDevice->device(), m_memory, nullptr);
}

void DixImage::createImage(
    EngineDevice& device,
    VkFormat format,
    uint32_t width,
    uint32_t height,
    VkImageTiling tiling,
    VkImageUsageFlags usage)
{
    VkImageCreateInfo imgInfo{};
    imgInfo.sType      = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType  = VK_IMAGE_TYPE_2D;
    imgInfo.extent     = { width, height, 1 };
    imgInfo.mipLevels  = 1;
    imgInfo.arrayLayers = 1;
    imgInfo.format     = format;
    imgInfo.tiling     = tiling;
    imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgInfo.usage      = usage;
    imgInfo.samples    = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    checkVulkanResult(vkCreateImage(device.device(), &imgInfo, nullptr, &m_image),
                      "Failed to create image");
}

void DixImage::allocateMemory(
    EngineDevice& dixDevice,
    VkMemoryPropertyFlags properties) {

    VkMemoryRequirements memReq{};
    vkGetImageMemoryRequirements(dixDevice.device(), m_image, &memReq);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memReq.size;
    allocInfo.memoryTypeIndex = dixDevice.findMemoryType(memReq.memoryTypeBits, properties);

    checkVulkanResult(vkAllocateMemory(dixDevice.device(), &allocInfo, nullptr, &m_memory),
                      "Failed to allocate image memory");

    vkBindImageMemory(dixDevice.device(), m_image, m_memory, 0);
}

void DixImage::createImageView(EngineDevice& device, VkFormat format) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image        = m_image;
    viewInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format       = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount   = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount    = 1;

    checkVulkanResult(vkCreateImageView(device.device(), &viewInfo, nullptr, &m_view),
                      "Failed to create image view");
}

std::tuple<VkImage, VkDeviceMemory, VkImageView> DixImage::releaseOwnership() {
    VkImage img = m_image;
    VkDeviceMemory mem = m_memory;
    VkImageView view = m_view;

    m_image = VK_NULL_HANDLE;
    m_memory = VK_NULL_HANDLE;
    m_view = VK_NULL_HANDLE;

    return std::make_tuple(img, mem, view);
}
}   // namespace dix