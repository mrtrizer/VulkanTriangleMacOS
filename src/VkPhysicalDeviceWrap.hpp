#pragma once

#include <vulkan/vulkan.h>

#include <limits>
#include <vector>
#include <unordered_set>

struct QueueFamilyIndices {
    unsigned graphicsFamily = std::numeric_limits<unsigned>::max();
    unsigned presentFamily = std::numeric_limits<unsigned>::max();
    
    bool isComplete() const {
        return graphicsFamily != std::numeric_limits<unsigned>::max()
                && presentFamily != std::numeric_limits<unsigned>::max();
    }
    
    std::vector<unsigned> indices() const {
        return {graphicsFamily, presentFamily};
    }
    
    std::unordered_set<unsigned> uniqueIndices() const {
        auto indicesVector = indices();
        return {indicesVector.begin(), indicesVector.end()};
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
    
    bool isComplete() {
        return !formats.empty() && !presentModes.empty();
    }
};

class VkPhysicalDeviceWrap {
public:
    VkPhysicalDeviceWrap(VkPhysicalDevice physicalDevice,
                         QueueFamilyIndices queueFamilies,
                         SwapChainSupportDetails supportDetails);
    
    //FIXME: Copy/move constructors
    
    ~VkPhysicalDeviceWrap() {
        // No need to destroy
    }
    
    const VkPhysicalDevice& physicalDevice() const { return m_physicalDevice; }
    const QueueFamilyIndices& queueFamilies() const { return m_queueFamilies; }
    const SwapChainSupportDetails& supportDetails() const { return m_supportDetails; }
    VkPhysicalDeviceMemoryProperties getMemoryProperties() const;
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    
private:
    VkPhysicalDevice m_physicalDevice;
    QueueFamilyIndices m_queueFamilies;
    SwapChainSupportDetails m_supportDetails;
};
