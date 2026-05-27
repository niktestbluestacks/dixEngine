#ifndef DEVICE_HPP
#define DEVICE_HPP

// dix
#include <Window/WindowClass/WindowClass.hpp>
#include <Utils/Class.hpp>
#include <Logger/Logger.hpp>

// std
#include <vector>
#include <vulkan/vulkan.hpp>

namespace dix {

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

class EngineDevice {
public:
// #if !defined(NDEBUG) || !defined(_WIN32)
//     const bool enableValidationLayers = false;
// #else
//     const bool enableValidationLayers = true;
// #endif

    const bool enableValidationLayers = true;

    EngineDevice(Window& window);
    ~EngineDevice();

    // Not copyable or movable
    DIX_DISABLE_COPY_AND_MOVE(EngineDevice);

    vk::CommandPool getCommandPool() { return commandPool; }
    vk::Device device() { return device_; }
    vk::SurfaceKHR surface() { return surface_; }
    vk::Queue graphicsQueue() { return graphicsQueue_; }
    vk::Queue presentQueue() { return presentQueue_; }

    SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
    vk::Format findSupportedFormat(
        const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

    // Buffer Helper Functions
    void createBuffer(
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties,
        vk::Buffer& buffer,
        vk::DeviceMemory& bufferMemory
    );
    vk::CommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer);
    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
    void copyBufferToImage(
        vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount = 1);

    void createImageWithInfo(
        const vk::ImageCreateInfo& imageInfo,
        vk::MemoryPropertyFlags properties,
        vk::Image& image,
        vk::DeviceMemory& imageMemory);

    const vk::PhysicalDeviceProperties& getProperties() const {
        return properties;
    }

    void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

    void recreateSurface();
    vk::PhysicalDevice getPhysicalDevice() const { return physicalDevice; }

    size_t getMaximumAllocationSize() const {
        return static_cast <size_t> (properties.limits.maxMemoryAllocationCount);
    }
private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    // helper functions
    bool isDeviceSuitable(vk::PhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
    void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
    void hasGflwRequiredInstanceExtensions();
    bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);

    vk::PhysicalDeviceProperties properties;
    vk::Instance instance;
    vk::DebugUtilsMessengerEXT debugMessenger;
    vk::PhysicalDevice physicalDevice = nullptr;
    Window& window;
    vk::CommandPool commandPool;

    vk::Device device_;
    vk::SurfaceKHR surface_;
    vk::Queue graphicsQueue_;
    vk::Queue presentQueue_;

    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

} // namespace dix

#endif // DEVICE_HPP