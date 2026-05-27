#ifndef FRAME_INFO_HPP
#define FRAME_INFO_HPP

// dix
#include <DixCamera/DixCamera.hpp>

// lib
#include <vulkan/vulkan.hpp>

namespace dix {
struct FrameInfo {
    int frameIndex;
    float frameTime;
    vk::CommandBuffer commandBuffer;
    DixCamera& dixcamera;
    vk::DescriptorSet globalDescriptorSet;
    vk::Extent2D screenExtent;
};
}  // namespace dix

#endif  // FRAME_INFO_HPP
