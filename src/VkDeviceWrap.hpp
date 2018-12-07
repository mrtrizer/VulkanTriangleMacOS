#pragma once

#include "VkPhysicalDeviceWrap.hpp"

class VkDeviceWrap {
public:
    VkDeviceWrap(const VkPhysicalDeviceWrap& physicalDevice,
                 const VkPhysicalDeviceFeatures& deviceFeatures,
                 const std::vector<const char*>& validationLayerNames,
                 const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
                 const std::vector<const char*>& extensionNames);
    
    ~VkDeviceWrap();
    
    VkDevice device() { return m_device; }

private:
    VkDevice m_device;
};
