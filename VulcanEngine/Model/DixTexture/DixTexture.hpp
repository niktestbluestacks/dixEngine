#ifndef DIXTEXTURE_HPP
#define DIXTEXTURE_HPP

// dix
#include <Pipeline/Buffer/DixBuffer.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Model/DixImage/DixImage.hpp>

// libs
#include <vulkan/vulkan.hpp>

namespace dix {

class DixTexture {
public:
    DixTexture();
    ~DixTexture();
    DixTexture(VkImage, VkDeviceMemory, VkImageView, VkSampler);
    void Destroy(EngineDevice device);

    VkImage& getImage() { return m_image; }
    VkImageView& getImageView() { return m_view; }
    VkDeviceMemory& getDeviceMemory() { return m_memory; }
    VkSampler& getSampler() { return m_sampler; }
    // const accessors
    const VkImage& getImage() const { return m_image; }
    const VkImageView& getImageView() const { return m_view; }
    const VkDeviceMemory& getDeviceMemory() const { return m_memory; }
    const VkSampler& getSampler() const { return m_sampler; }

private:
    VkImage m_image;
    VkDeviceMemory m_memory;
    VkImageView m_view;
    VkSampler m_sampler;
};

DixTexture createDefaultTexture(EngineDevice &dixDevice);
DixTexture createTextureFromFile(const std::string& path, EngineDevice& dixDevice);

}   // namespace dix

#endif // DIXTEXTURE_HPP