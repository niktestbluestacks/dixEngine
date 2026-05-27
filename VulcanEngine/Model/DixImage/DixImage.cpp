// dix
#include <Model/DixImage/DixImage.hpp>

namespace dix {
static void checkVulkanResult(vk::Result res, const char* msg) {
    if (res != vk::Result::eSuccess) {
        throw std::runtime_error(std::string(msg) + " (VK error " +
                                 std::to_string(static_cast<int>(res)) + ")");
    }
}

DixImage::DixImage()
    : m_image(nullptr),
      m_memory(nullptr),
      m_view(nullptr),
      m_height(0),
      m_width(0) {}

DixImage::DixImage(EngineDevice& dixDevice, vk::Format format, uint32_t width,
                   uint32_t height, vk::ImageTiling tiling,
                   vk::ImageUsageFlags usage,
                   vk::MemoryPropertyFlags properties)
    : m_width(width), m_height(height), m_dixDevice(&dixDevice) {
    createImage(dixDevice, format, width, height, tiling, usage);

    allocateMemory(dixDevice, properties);

    createImageView(dixDevice, format);
}

DixImage::~DixImage() {
    if (m_view) m_dixDevice->device().destroyImageView(m_view);
    if (m_image) m_dixDevice->device().destroyImage(m_image);
    if (m_memory) m_dixDevice->device().freeMemory(m_memory);
}

void DixImage::createImage(EngineDevice& device, vk::Format format,
                           uint32_t width, uint32_t height,
                           vk::ImageTiling tiling, vk::ImageUsageFlags usage) {
    vk::ImageCreateInfo imgInfo{};
    imgInfo.imageType = vk::ImageType::e2D;
    imgInfo.extent = vk::Extent3D{width, height, 1};
    imgInfo.mipLevels = 1;
    imgInfo.arrayLayers = 1;
    imgInfo.format = format;
    imgInfo.tiling = tiling;
    imgInfo.initialLayout = vk::ImageLayout::eUndefined;
    imgInfo.usage = usage;
    imgInfo.samples = vk::SampleCountFlagBits::e1;
    imgInfo.sharingMode = vk::SharingMode::eExclusive;

    m_image = device.device().createImage(imgInfo, nullptr);
}

void DixImage::allocateMemory(EngineDevice& dixDevice,
                              vk::MemoryPropertyFlags properties) {
    vk::MemoryRequirements memReq{};
    dixDevice.device().getImageMemoryRequirements(m_image, &memReq);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex =
        dixDevice.findMemoryType(memReq.memoryTypeBits, properties);

    m_memory = dixDevice.device().allocateMemory(allocInfo, nullptr);

    dixDevice.device().bindImageMemory(m_image, m_memory, 0);
}

void DixImage::createImageView(EngineDevice& device, vk::Format format) {
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.image = m_image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    m_view = device.device().createImageView(viewInfo, nullptr);
}

std::tuple<vk::Image, vk::DeviceMemory, vk::ImageView>
DixImage::releaseOwnership() {
    vk::Image img = m_image;
    vk::DeviceMemory mem = m_memory;
    vk::ImageView view = m_view;

    m_image = nullptr;
    m_memory = nullptr;
    m_view = nullptr;

    return std::make_tuple(img, mem, view);
}
}  // namespace dix