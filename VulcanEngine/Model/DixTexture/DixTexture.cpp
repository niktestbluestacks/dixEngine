// dix
#include <Logger/Logger.hpp>
#include <Model/DixTexture/DixTexture.hpp>


// libs
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// std
#include <filesystem>
#include <stdexcept>


namespace dix {
DixTexture::DixTexture() {
    m_image = nullptr;
    m_memory = nullptr;
    m_sampler = nullptr;
    m_view = nullptr;
}

DixTexture::~DixTexture() = default;

DixTexture::DixTexture(vk::Image image, vk::DeviceMemory memory,
                       vk::ImageView view, vk::Sampler sampler)
    : m_image(image), m_memory(memory), m_view(view), m_sampler(sampler) {}

DixTexture createDefaultTexture(EngineDevice& dixDevice) {
    const int texWidth = 1;
    const int texHeight = 1;
    const vk::Format texFormat = vk::Format::eR8G8B8A8Unorm;

    // 1×1 white pixel
    uint8_t pixel[4] = {255, 255, 255, 255};

    // 1. create staging buffer
    DixBuffer staging{dixDevice, 4, 1, vk::BufferUsageFlagBits::eTransferSrc,
                      vk::MemoryPropertyFlagBits::eHostVisible |
                          vk::MemoryPropertyFlagBits::eHostCoherent};
    staging.map();
    staging.writeToBuffer(pixel);

    // 2. create image
    DixImage tex{
        dixDevice,
        texFormat,
        texWidth,
        texHeight,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal};

    // 3. transition & copy
    dixDevice.transitionImageLayout(tex.getImage(), vk::ImageLayout::eUndefined,
                                    vk::ImageLayout::eTransferDstOptimal);
    dixDevice.copyBufferToImage(staging.getBuffer(), tex.getImage(), texWidth,
                                texHeight);
    dixDevice.transitionImageLayout(tex.getImage(),
                                    vk::ImageLayout::eTransferDstOptimal,
                                    vk::ImageLayout::eShaderReadOnlyOptimal);

    // 4. take ownership of image/view/memory from DixImage
    vk::Image viewImage;
    vk::DeviceMemory viewMemory;
    vk::ImageView view;
    std::tie(viewImage, viewMemory, view) = tex.releaseOwnership();

    // 5. create sampler
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    vk::Sampler sampler = dixDevice.device().createSampler(samplerInfo);

    return {viewImage, viewMemory, view, sampler};
}

DixTexture createTextureFromFile(const std::string& path,
                                 EngineDevice& dixDevice) {
    // Debug: log texture path
    DixLogDebug("Loading texture from: {}", path);

    // Verify file exists
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("Texture file not found: " + path);
    }

    int texWidth, texHeight, texChannels;
    const vk::Format texFormat = vk::Format::eR8G8B8A8Unorm;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight,
                                &texChannels, STBI_rgb_alpha);
    if (!pixels) throw std::runtime_error("Failed to load texture: " + path);

    const vk::DeviceSize imageSize = texWidth * texHeight * 4;

    vk::DeviceSize instanceSize = 4;  // RGBA8

    // 1. staging buffer
    DixBuffer staging{dixDevice, instanceSize, static_cast<uint32_t>(imageSize),
                      vk::BufferUsageFlagBits::eTransferSrc,
                      vk::MemoryPropertyFlagBits::eHostVisible |
                          vk::MemoryPropertyFlagBits::eHostCoherent};
    staging.map();
    staging.writeToBuffer(pixels, imageSize);
    stbi_image_free(pixels);

    // 2. image
    DixImage tex{
        dixDevice,
        vk::Format::eR8G8B8A8Unorm,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight),
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal};

    // 3. transition & copy
    dixDevice.transitionImageLayout(tex.getImage(), vk::ImageLayout::eUndefined,
                                    vk::ImageLayout::eTransferDstOptimal);
    dixDevice.copyBufferToImage(staging.getBuffer(), tex.getImage(), texWidth,
                                texHeight);
    dixDevice.transitionImageLayout(tex.getImage(),
                                    vk::ImageLayout::eTransferDstOptimal,
                                    vk::ImageLayout::eShaderReadOnlyOptimal);

    // 4. take ownership of image/view/memory from DixImage
    vk::Image viewImage;
    vk::DeviceMemory viewMemory;
    vk::ImageView view;
    std::tie(viewImage, viewMemory, view) = tex.releaseOwnership();

    // 5. create sampler
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    vk::Sampler sampler = dixDevice.device().createSampler(samplerInfo);

    return {viewImage, viewMemory, view, sampler};
}
}  // namespace dix