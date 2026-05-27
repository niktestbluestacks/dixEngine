// dix
#include <Pipeline/SwapChain/SwapChain.hpp>

#include <Logger/Logger.hpp>

// libs
#include <vulkan/vk_enum_string_helper.h>

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace dix {

SwapChain::SwapChain(EngineDevice& deviceRef, vk::Extent2D extent)
    : device{ deviceRef }, windowExtent{ extent } {
    init();
}

SwapChain::SwapChain(EngineDevice& deviceRef, vk::Extent2D extent, std::shared_ptr <SwapChain> previous)
    : device{ deviceRef }, windowExtent{ extent }, oldSwapChain{ previous } {
    init();

    // clean up old swap chain since it's no longer needed
    oldSwapChain = nullptr;
}

void SwapChain::init() {
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
}

SwapChain::~SwapChain() {
    for (auto imageView : swapChainImageViews) {
        device.device().destroyImageView(imageView);
    }
    swapChainImageViews.clear();

    if (swapChain != nullptr) {
        device.device().destroySwapchainKHR(swapChain);
        swapChain = nullptr;
    }

    for (int i = 0; i < depthImages.size(); i++) {
        device.device().destroyImageView(depthImageViews[i]);
        device.device().destroyImage(depthImages[i]);
        device.device().freeMemory(depthImageMemorys[i]);
    }

    for (auto framebuffer : swapChainFramebuffers) {
        device.device().destroyFramebuffer(framebuffer);
    }

    device.device().destroyRenderPass(renderPass);

    // cleanup synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        device.device().destroySemaphore(renderFinishedSemaphores[i]);
        device.device().destroySemaphore(imageAvailableSemaphores[i]);
        device.device().destroyFence(inFlightFences[i]);
    }
}

vk::Result SwapChain::acquireNextImage(uint32_t* imageIndex) {
    auto res1 = device.device().waitForFences({inFlightFences[currentFrame]}, vk::True, std::numeric_limits<uint64_t>::max());

    vk::Result result = device.device().acquireNextImageKHR(
        swapChain,
        std::numeric_limits<uint64_t>::max(),
        imageAvailableSemaphores[currentFrame],  // must be a not signaled semaphore
        nullptr,
        imageIndex);

    return result;
}

vk::Result SwapChain::submitCommandBuffers(
    const vk::CommandBuffer* buffers, uint32_t* imageIndex) {
    if (imagesInFlight[*imageIndex] != nullptr) {
        auto res2 =device.device().waitForFences({imagesInFlight[*imageIndex]}, vk::True, UINT64_MAX);
    }
    imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

    vk::Semaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    vk::SubmitInfo submitInfo{};
    submitInfo.setWaitSemaphoreCount(1)
              .setPWaitSemaphores(waitSemaphores)
              .setPWaitDstStageMask(waitStages)
              .setCommandBufferCount(1)
              .setPCommandBuffers(buffers)
              .setSignalSemaphoreCount(1)
              .setPSignalSemaphores(&renderFinishedSemaphores[currentFrame]);

    device.device().resetFences(inFlightFences[currentFrame]);
    device.graphicsQueue().submit(submitInfo, inFlightFences[currentFrame]);

    vk::PresentInfoKHR presentInfo{};
    presentInfo.setWaitSemaphoreCount(1)
               .setPWaitSemaphores(&renderFinishedSemaphores[currentFrame])
               .setSwapchainCount(1)
               .setPSwapchains(&swapChain)
               .setPImageIndices(imageIndex);

    auto result = device.presentQueue().presentKHR(presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}

void SwapChain::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = device.getSwapChainSupport();

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.setSurface(device.surface())
              .setMinImageCount(imageCount)
              .setImageFormat(surfaceFormat.format)
              .setImageColorSpace(surfaceFormat.colorSpace)
              .setImageExtent(extent)
              .setImageArrayLayers(1)
              .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

    QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
                  .setQueueFamilyIndexCount(2)
                  .setPQueueFamilyIndices(queueFamilyIndices);
    }
    else {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }

    createInfo.setPreTransform(swapChainSupport.capabilities.currentTransform)
              .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
              .setPresentMode(presentMode)
              .setClipped(vk::True)
              .setOldSwapchain(oldSwapChain == nullptr ? nullptr : oldSwapChain->swapChain);

    try {
        swapChain = device.device().createSwapchainKHR(createInfo);
    } catch (...) {
        throw std::runtime_error("failed to create swap chain!");
    }

    // we only specified a minimum number of images in the swap chain, so the implementation is
    // allowed to create a swap chain with more. That's why we'll first query the final number of
    // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
    // retrieve the handles.
    swapChainImages = device.device().getSwapchainImagesKHR(swapChain);

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void SwapChain::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.setImage(swapChainImages[i])
              .setViewType(vk::ImageViewType::e2D)
              .setFormat(swapChainImageFormat)
              .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

        try {
            swapChainImageViews[i] = device.device().createImageView(viewInfo);
        } catch (...) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }
}

void SwapChain::createRenderPass() {
    vk::AttachmentDescription depthAttachment{};
    depthAttachment.setFormat(findDepthFormat())
                   .setSamples(vk::SampleCountFlagBits::e1)
                   .setLoadOp(vk::AttachmentLoadOp::eClear)
                   .setStoreOp(vk::AttachmentStoreOp::eDontCare)
                   .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                   .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                   .setInitialLayout(vk::ImageLayout::eUndefined)
                   .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::AttachmentReference depthAttachmentRef{};
    depthAttachmentRef.setAttachment(1)
                      .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::AttachmentDescription colorAttachment{};
    colorAttachment.setFormat(getSwapChainImageFormat())
                   .setSamples(vk::SampleCountFlagBits::e1)
                   .setLoadOp(vk::AttachmentLoadOp::eClear)
                   .setStoreOp(vk::AttachmentStoreOp::eStore)
                   .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                   .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                   .setInitialLayout(vk::ImageLayout::eUndefined)
                   .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachmentRef{};
    colorAttachmentRef.setAttachment(0)
                      .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass{};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
           .setColorAttachments({colorAttachmentRef})
           .setPDepthStencilAttachment(&depthAttachmentRef);

    vk::SubpassDependency dependency{};
    dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
              .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
              .setDstSubpass(0)
              .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
              .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.setAttachments(attachments)
                  .setSubpasses({subpass})
                  .setDependencies({dependency});

    try {
        renderPass = device.device().createRenderPass(renderPassInfo);
    } catch (...) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void SwapChain::createFramebuffers() {
    swapChainFramebuffers.resize(imageCount());
    for (size_t i = 0; i < imageCount(); i++) {
        std::array<vk::ImageView, 2> attachments = { swapChainImageViews[i], depthImageViews[i] };

        vk::Extent2D swapChainExtent = getSwapChainExtent();
        vk::FramebufferCreateInfo framebufferInfo{};
        framebufferInfo.setRenderPass(renderPass)
                       .setAttachments(attachments)
                       .setWidth(swapChainExtent.width)
                       .setHeight(swapChainExtent.height)
                       .setLayers(1);

        try {
            swapChainFramebuffers[i] = device.device().createFramebuffer(framebufferInfo);
        } catch (...) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void SwapChain::createDepthResources() {
    vk::Format depthFormat = findDepthFormat();
    swapChainDepthFormat = depthFormat;
    vk::Extent2D swapChainExtent = getSwapChainExtent();

    depthImages.resize(imageCount());
    depthImageMemorys.resize(imageCount());
    depthImageViews.resize(imageCount());

    for (int i = 0; i < depthImages.size(); i++) {
        vk::ImageCreateInfo imageInfo{};
        imageInfo.setImageType(vk::ImageType::e2D)
                   .setExtent({swapChainExtent.width, swapChainExtent.height, 1})
                   .setMipLevels(1)
                   .setArrayLayers(1)
                   .setFormat(depthFormat)
                   .setTiling(vk::ImageTiling::eOptimal)
                   .setInitialLayout(vk::ImageLayout::eUndefined)
                   .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
                   .setSamples(vk::SampleCountFlagBits::e1)
                   .setSharingMode(vk::SharingMode::eExclusive);

        device.createImageWithInfo(
            imageInfo,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            depthImages[i],
            depthImageMemorys[i]);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.setImage(depthImages[i])
              .setViewType(vk::ImageViewType::e2D)
              .setFormat(depthFormat)
              .setSubresourceRange({vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1});

        try {
            depthImageViews[i] = device.device().createImageView(viewInfo);
        } catch (...) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }
}

void SwapChain::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(imageCount(), nullptr);

    vk::SemaphoreCreateInfo semaphoreInfo{};

    vk::FenceCreateInfo fenceInfo{};
    fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        try {
            imageAvailableSemaphores[i] = device.device().createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = device.device().createSemaphore(semaphoreInfo);
            inFlightFences[i] = device.device().createFence(fenceInfo);
        } catch (...) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR SwapChain::chooseSwapPresentMode(
        const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    // for (const auto& availablePresentMode : availablePresentModes) {
    //     if (availablePresentMode == VK_PRESENT_MODE_FIFO_LATEST_READY_EXT) {
    //         DixLogInfo("Present mode: Fifo latest ready ext");
    //         return availablePresentMode;
    //     }
    // }


    for (const auto &availablePresentMode : availablePresentModes) {
       if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
           DixLogInfo("Present mode: Immediate");
           return availablePresentMode;
       }
    }
    DixLogInfo("Present mode: V-Sync");
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D SwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        vk::Extent2D actualExtent = windowExtent;
        actualExtent.width = std::max(
            capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(
            capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

vk::Format SwapChain::findDepthFormat() {
    return device.findSupportedFormat(
        { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

}  // namespace dix