#ifndef DIXTEXTURE_HPP
#define DIXTEXTURE_HPP

// dix
#include <Model/DixImage/DixImage.hpp>
#include <Pipeline/Buffer/DixBuffer.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>

// libs
#include <vulkan/vulkan.hpp>

namespace dix {

class DixTexture {
   public:
    DixTexture();
    ~DixTexture();
    DixTexture(vk::Image, vk::DeviceMemory, vk::ImageView, vk::Sampler);
    void Destroy(EngineDevice device);

    vk::Image& getImage() { return m_image; }
    vk::ImageView& getImageView() { return m_view; }
    vk::DeviceMemory& getDeviceMemory() { return m_memory; }
    vk::Sampler& getSampler() { return m_sampler; }
    // const accessors
    const vk::Image& getImage() const { return m_image; }
    const vk::ImageView& getImageView() const { return m_view; }
    const vk::DeviceMemory& getDeviceMemory() const { return m_memory; }
    const vk::Sampler& getSampler() const { return m_sampler; }

   private:
    vk::Image m_image;
    vk::DeviceMemory m_memory;
    vk::ImageView m_view;
    vk::Sampler m_sampler;
};

DixTexture createDefaultTexture(EngineDevice& dixDevice);
DixTexture createTextureFromFile(const std::string& path,
                                 EngineDevice& dixDevice);

}  // namespace dix

#endif  // DIXTEXTURE_HPP