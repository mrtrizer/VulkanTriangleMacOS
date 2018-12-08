#pragma once

#include <vector>

#include <vulkan/vulkan.h>

struct SwapchainSettings {
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    const VkExtent2D& extent;
    uint32_t imageCount;
    VkSurfaceTransformFlagBitsKHR transform;
};

class VkDeviceWrap;
class VkSurfaceWrap;

class VkSwapchainWrap {
public:
    VkSwapchainWrap(const VkDeviceWrap& device, const VkSurfaceWrap& surface, const SwapchainSettings& settings);
    
    ~VkSwapchainWrap();
    
    VkSwapchainKHR swapchain() const { return m_swapchain; }
    const VkDeviceWrap& device() const { return m_device; }
    std::vector<VkImage> getSwapchainImages() const;
    
private:
    VkSwapchainKHR m_swapchain;
    const VkDeviceWrap& m_device;
};
