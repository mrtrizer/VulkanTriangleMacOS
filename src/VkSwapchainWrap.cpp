#include "VkSwapchainWrap.hpp"
#include "VkDeviceWrap.hpp"
#include "VkSurfaceWrap.hpp"

VkSwapchainWrap::VkSwapchainWrap(const VkDeviceWrap& device, const VkSurfaceWrap& surface, const SwapchainSettings& settings)
    : m_device(device)
{
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface.surface();
    createInfo.minImageCount = settings.imageCount;
    createInfo.imageFormat = settings.surfaceFormat.format;
    createInfo.imageColorSpace = settings.surfaceFormat.colorSpace;
    createInfo.imageExtent = settings.extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    auto queueFamilies = device.physicalDevice().queueFamilies();
    auto queueFamilyIndices = device.physicalDevice().queueFamilies().indices();
    
    if (queueFamilies.graphicsFamily != queueFamilies.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
        createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    
    createInfo.preTransform = settings.transform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    
    createInfo.presentMode = settings.presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }
}

VkSwapchainWrap::~VkSwapchainWrap() {
    vkDestroySwapchainKHR(m_device.device(), m_swapchain, nullptr);
}

std::vector<VkImage> VkSwapchainWrap::getSwapchainImages() const {
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &imageCount, nullptr);
    std::vector<VkImage> swapchainImages(imageCount);
    vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &imageCount, swapchainImages.data());
    return swapchainImages;
}
