#ifndef FRAME_INFO_HPP
#define FRAME_INFO_HPP

// dix
#include <DixCamera/DixCamera.hpp>

// lib
#include <vulkan/vulkan.h>

namespace dix {
struct FrameInfo {
	int frameIndex;
	float frameTime;
	VkCommandBuffer commandBuffer;
	DixCamera& dixcamera;
	VkDescriptorSet globalDescriptorSet;
};
}	// namespace dix

#endif // FRIEND_INFO_HPP
