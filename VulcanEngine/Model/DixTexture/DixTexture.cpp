// dix
#include <Model/DixTexture/DixTexture.hpp>
#include <Logger/Logger.hpp>

// libs
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif // STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// std
#include <stdexcept>
#include <filesystem>

namespace dix {
DixTexture::DixTexture() {
    m_image = VK_NULL_HANDLE;
    m_memory = VK_NULL_HANDLE;
    m_sampler = VK_NULL_HANDLE;
    m_view = VK_NULL_HANDLE;
}

DixTexture::~DixTexture() = default;

DixTexture::DixTexture(VkImage image, VkDeviceMemory memory, VkImageView view, VkSampler sampler):
        m_image(image),
        m_memory(memory),
        m_view(view),
        m_sampler(sampler) {}

DixTexture createDefaultTexture(EngineDevice& dixDevice) {
    const int texWidth  = 1;
    const int texHeight = 1;
    const VkFormat texFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // 1×1 white pixel
    uint8_t pixel[4] = {255, 255, 255, 255};

    // 1. create staging buffer
    DixBuffer staging{dixDevice, 4, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
    staging.map();
    staging.writeToBuffer(pixel);

    // 2. create image
    DixImage tex{dixDevice, texFormat, texWidth, texHeight,
                 VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

    // 3. transition & copy
    dixDevice.transitionImageLayout(tex.getImage(), VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    dixDevice.copyBufferToImage(staging.getBuffer(), tex.getImage(),
                             texWidth, texHeight);
    dixDevice.transitionImageLayout(tex.getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // 4. take ownership of image/view/memory from DixImage
    VkImage viewImage;
    VkDeviceMemory viewMemory;
    VkImageView view;
    std::tie(viewImage, viewMemory, view) = tex.releaseOwnership();

    // 5. create sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkSampler sampler;
    vkCreateSampler(dixDevice.device(), &samplerInfo, nullptr, &sampler);

    return { viewImage, viewMemory, view, sampler };
}       

DixTexture createTextureFromFile(const std::string& path, EngineDevice& dixDevice) {
    // Debug: log texture path
    DixLogDebug("Loading texture from: " + path);
    
    // Verify file exists
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("Texture file not found: " + path);
    }
    
    int texWidth, texHeight, texChannels;
    const VkFormat texFormat = VK_FORMAT_R8G8B8A8_UNORM;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels) throw std::runtime_error("Failed to load texture: " + path);

    const VkDeviceSize imageSize = texWidth * texHeight * 4;

    VkDeviceSize instanceSize = 4; // RGBA8

    // 1. staging buffer
    DixBuffer staging{dixDevice, instanceSize, static_cast<uint32_t>(imageSize),
                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
    staging.map();
    staging.writeToBuffer(pixels, imageSize);
    stbi_image_free(pixels);

    // 2. image
    DixImage tex{dixDevice, VK_FORMAT_R8G8B8A8_UNORM,
                 static_cast <uint32_t>(texWidth), static_cast <uint32_t>(texHeight),
                 VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

    // 3. transition & copy
    dixDevice.transitionImageLayout(tex.getImage(), VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    dixDevice.copyBufferToImage(staging.getBuffer(), tex.getImage(),
                             texWidth, texHeight);
    dixDevice.transitionImageLayout(tex.getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // 4. take ownership of image/view/memory from DixImage
    VkImage viewImage;
    VkDeviceMemory viewMemory;
    VkImageView view;
    std::tie(viewImage, viewMemory, view) = tex.releaseOwnership();

    // 5. create sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkSampler sampler;
    if (vkCreateSampler(
        dixDevice.device(),
        &samplerInfo,
        nullptr,
        &sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create sampler for texture: " + path);
    }

    return { viewImage, viewMemory, view, sampler };
}
}   // namespace dix