// dix
#include <Logger/Logger.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>

// libs
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_to_string.hpp>

// std
#include <cstring>
#include <set>
#include <string>
#include <unordered_set>

namespace dix {

// local callback functions
static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              vk::DebugUtilsMessageTypeFlagsEXT messageType,
              const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
              void* pUserData) {
    const char* messageId = pCallbackData->pMessageIdName
                                ? pCallbackData->pMessageIdName
                                : "UNKNOWN";

    switch (messageSeverity) {
        case (vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose):
        case (vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo):
            DixLogInfo("validation layer: {}, {}, message ID: {}, message:",
                       vk::to_string(messageSeverity),
                       vk::to_string(messageType), messageId,
                       pCallbackData->pMessage);
            break;
        case (vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning):
            DixLogWarn("validation layer: {}, {}, message ID: {}, message:",
                       vk::to_string(messageSeverity),
                       vk::to_string(messageType), messageId,
                       pCallbackData->pMessage);
            break;
        case (vk::DebugUtilsMessageSeverityFlagBitsEXT::eError):
            DixLogErr("validation layer: {}, {}, message ID: {}, message:",
                      vk::to_string(messageSeverity),
                      vk::to_string(messageType), messageId,
                      pCallbackData->pMessage);
            break;
    }

    return VK_FALSE;
}

vk::Result CreateDebugUtilsMessengerEXT(
    vk::Instance instance,
    const vk::DebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    vk::DebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        static_cast<VkInstance>(instance), "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return static_cast<vk::Result>(
            func(static_cast<VkInstance>(instance),
                 reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(
                     pCreateInfo),
                 pAllocator,
                 reinterpret_cast<VkDebugUtilsMessengerEXT*>(pDebugMessenger)));
    } else {
        return vk::Result::eErrorExtensionNotPresent;
    }
}

void DestroyDebugUtilsMessengerEXT(vk::Instance instance,
                                   vk::DebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        static_cast<VkInstance>(instance), "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(static_cast<VkInstance>(instance),
             static_cast<VkDebugUtilsMessengerEXT>(debugMessenger), pAllocator);
    }
}

// class member functions
EngineDevice::EngineDevice(Window& window) : window{window} {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
}

EngineDevice::~EngineDevice() {
    device_.destroyCommandPool(commandPool);
    device_.destroy();

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    instance.destroySurfaceKHR(surface_);
    instance.destroy();
}

void EngineDevice::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error(
            "validation layers requested, but not available!");
    }

    vk::ApplicationInfo appInfo{};
    appInfo.setPApplicationName("LittleVulkanEngine App")
        .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
        .setPEngineName("No Engine")
        .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
        .setApiVersion(VK_API_VERSION_1_0);

    vk::InstanceCreateInfo createInfo{};
    createInfo.setPApplicationInfo(&appInfo);

    auto extensions = getRequiredExtensions();
    createInfo
        .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
        .setPpEnabledExtensionNames(extensions.data());

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.setPNext(
            reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(
                &debugCreateInfo));
    } else {
        createInfo.setPNext(nullptr);
    }

    instance = vk::createInstance(createInfo);

    hasGflwRequiredInstanceExtensions();
}

void EngineDevice::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    std::vector<vk::PhysicalDevice> devices =
        instance.enumeratePhysicalDevices();
    if (devices.empty()) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    if (enableValidationLayers) {
        DixLogDebug("Device count: ");
    }

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (!physicalDevice) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    properties = physicalDevice.getProperties();
    if (enableValidationLayers) {
        DixLogDebug("physical device: ", properties.deviceName);
    }
}

void EngineDevice::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily,
                                              indices.presentFamily};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.setQueueFamilyIndex(queueFamily)
            .setQueueCount(1)
            .setPQueuePriorities(&queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    vk::DeviceCreateInfo createInfo{};
    createInfo
        .setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
        .setPQueueCreateInfos(queueCreateInfos.data())
        .setPEnabledFeatures(&deviceFeatures)
        .setEnabledExtensionCount(
            static_cast<uint32_t>(deviceExtensions.size()))
        .setPpEnabledExtensionNames(deviceExtensions.data());

    // device specific validation layers have been deprecated, so we don't set
    // enabled layer count anymore

    device_ = physicalDevice.createDevice(createInfo);

    graphicsQueue_ = device_.getQueue(indices.graphicsFamily, 0);
    presentQueue_ = device_.getQueue(indices.presentFamily, 0);
}

void EngineDevice::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.setQueueFamilyIndex(queueFamilyIndices.graphicsFamily)
        .setFlags(vk::CommandPoolCreateFlagBits::eTransient |
                  vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

    commandPool = device_.createCommandPool(poolInfo);
}

void EngineDevice::createSurface() {
    window.createWindowSurface(instance, &surface_);
}

void EngineDevice::recreateSurface() {
    instance.destroySurfaceKHR(surface_);
    createSurface();
}

bool EngineDevice::isDeviceSuitable(vk::PhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport =
            querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() &&
                            !swapChainSupport.presentModes.empty();
    }

    vk::PhysicalDeviceFeatures supportedFeatures = device.getFeatures();

    return indices.isComplete() && extensionsSupported && swapChainAdequate &&
           supportedFeatures.samplerAnisotropy;
}

void EngineDevice::populateDebugMessengerCreateInfo(
    vk::DebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = vk::DebugUtilsMessengerCreateInfoEXT{};
    createInfo.setMessageSeverity(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    createInfo.setMessageType(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);
    createInfo.setPfnUserCallback(debugCallback);
    createInfo.setPUserData(nullptr);  // Optional
}

void EngineDevice::setupDebugMessenger() {
    if (!enableValidationLayers) return;
    vk::DebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);
    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
                                     &debugMessenger) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

bool EngineDevice::checkValidationLayerSupport() {
    std::vector<vk::LayerProperties> availableLayers =
        vk::enumerateInstanceLayerProperties();

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<const char*> EngineDevice::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions,
                                        glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void EngineDevice::hasGflwRequiredInstanceExtensions() {
    std::vector<vk::ExtensionProperties> extensions =
        vk::enumerateInstanceExtensionProperties(nullptr);

    if (enableValidationLayers) {
        DixLogDebug("available extensions:");
    }
    std::unordered_set<std::string> available;
    for (const auto& extension : extensions) {
        if (enableValidationLayers) {
            DixLogDebug("\t{}",
                        static_cast<std::string>(extension.extensionName));
        }
        available.insert(extension.extensionName);
    }

    if (enableValidationLayers) {
        DixLogDebug("required extensions:");
    }
    auto requiredExtensions = getRequiredExtensions();
    for (const auto& required : requiredExtensions) {
        if (enableValidationLayers) {
            DixLogDebug("\t{}", required);
        }
        if (available.find(required) == available.end()) {
            throw std::runtime_error("Missing required glfw extension");
        }
    }
}

bool EngineDevice::checkDeviceExtensionSupport(vk::PhysicalDevice device) {
    std::vector<vk::ExtensionProperties> availableExtensions =
        device.enumerateDeviceExtensionProperties(nullptr);

    std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                             deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices EngineDevice::findQueueFamilies(vk::PhysicalDevice device) {
    QueueFamilyIndices indices;

    std::vector<vk::QueueFamilyProperties> queueFamilies =
        device.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 &&
            queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
            indices.graphicsFamilyHasValue = true;
        }
        vk::Bool32 presentSupport =
            device.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface_);
        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
            indices.presentFamilyHasValue = true;
        }
        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

SwapChainSupportDetails EngineDevice::querySwapChainSupport(
    vk::PhysicalDevice device) {
    SwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(surface_);

    details.formats = device.getSurfaceFormatsKHR(surface_);

    details.presentModes = device.getSurfacePresentModesKHR(surface_);

    return details;
}

vk::Format EngineDevice::findSupportedFormat(
    const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
    vk::FormatFeatureFlags features) {
    for (vk::Format format : candidates) {
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal &&
                   (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}

uint32_t EngineDevice::findMemoryType(uint32_t typeFilter,
                                      vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties =
        physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void EngineDevice::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                vk::MemoryPropertyFlags properties,
                                vk::Buffer& buffer,
                                vk::DeviceMemory& bufferMemory) {
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.setSize(size).setUsage(usage).setSharingMode(
        vk::SharingMode::eExclusive);

    buffer = device_.createBuffer(bufferInfo);

    vk::MemoryRequirements memRequirements =
        device_.getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(
            findMemoryType(memRequirements.memoryTypeBits, properties));

    bufferMemory = device_.allocateMemory(allocInfo);

    device_.bindBufferMemory(buffer, bufferMemory, 0);
}

vk::CommandBuffer EngineDevice::beginSingleTimeCommands() {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandPool(commandPool)
        .setCommandBufferCount(1);

    vk::CommandBuffer commandBuffer =
        device_.allocateCommandBuffers(allocInfo)[0];

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    commandBuffer.begin(beginInfo);
    return commandBuffer;
}

void EngineDevice::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
    commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.setCommandBufferCount(1).setPCommandBuffers(&commandBuffer);

    graphicsQueue_.submit(submitInfo, {});
    graphicsQueue_.waitIdle();

    device_.freeCommandBuffers(commandPool, commandBuffer);
}

void EngineDevice::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer,
                              vk::DeviceSize size) {
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

    vk::BufferCopy copyRegion{};
    copyRegion.srcOffset = 0;  // Optional
    copyRegion.dstOffset = 0;  // Optional
    copyRegion.size = size;
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void EngineDevice::copyBufferToImage(vk::Buffer buffer, vk::Image image,
                                     uint32_t width, uint32_t height,
                                     uint32_t layerCount) {
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;

    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{width, height, 1};

    commandBuffer.copyBufferToImage(
        buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
    endSingleTimeCommands(commandBuffer);
}

void EngineDevice::createImageWithInfo(const vk::ImageCreateInfo& imageInfo,
                                       vk::MemoryPropertyFlags properties,
                                       vk::Image& image,
                                       vk::DeviceMemory& imageMemory) {
    image = device_.createImage(imageInfo);

    vk::MemoryRequirements memRequirements =
        device_.getImageMemoryRequirements(image);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(
            findMemoryType(memRequirements.memoryTypeBits, properties));

    imageMemory = device_.allocateMemory(allocInfo);

    device_.bindImageMemory(image, imageMemory, 0);
}

void EngineDevice::transitionImageLayout(vk::Image image,
                                         vk::ImageLayout oldLayout,
                                         vk::ImageLayout newLayout) {
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {},
                                  barrier);

    endSingleTimeCommands(commandBuffer);
}

}  // namespace dix