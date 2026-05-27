#ifndef DIX_IMAGE_HPP
#define DIX_IMAGE_HPP

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>

// libs
#include <vulkan/vulkan.hpp>

// std
#include <stdexcept>
#include <string>
#include <tuple>


namespace dix {
class DixImage {
   public:
    DixImage();

    DixImage(EngineDevice& dixDevice, vk::Format format, uint32_t width,
             uint32_t height, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
             vk::MemoryPropertyFlags properties);

    ~DixImage();

    vk::Image& getImage() { return m_image; };
    vk::DeviceMemory& getMemory() { return m_memory; };
    vk::ImageView& getImageView() { return m_view; };

    std::tuple<vk::Image, vk::DeviceMemory, vk::ImageView> releaseOwnership();

   private:
    void createImage(EngineDevice& device, vk::Format format, uint32_t width,
                     uint32_t height, vk::ImageTiling tiling,
                     vk::ImageUsageFlags usage);

    void allocateMemory(EngineDevice& device,
                        vk::MemoryPropertyFlags properties);

    void createImageView(EngineDevice& device, vk::Format format);

    vk::Image m_image;
    vk::DeviceMemory m_memory;
    vk::ImageView m_view;

    uint32_t m_width;
    uint32_t m_height;
    EngineDevice* m_dixDevice;
};
}  // namespace dix

#endif  // DIX_IMAGE_HPP