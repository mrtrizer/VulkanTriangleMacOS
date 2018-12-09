#pragma once

#include "VkPhysicalDeviceWrap.hpp"
#include "VkBufferWrap.hpp"
#include "HostBufferController.hpp"

class VkDeviceWrap {
public:
    VkDeviceWrap(const VkPhysicalDeviceWrap& physicalDevice,
                 const VkPhysicalDeviceFeatures& deviceFeatures,
                 const std::vector<const char*>& validationLayerNames,
                 const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
                 const std::vector<const char*>& extensionNames);
    
    ~VkDeviceWrap();
    
    VkDevice device() const { return m_device; }
    const VkPhysicalDeviceWrap& physicalDevice() const { return m_physicalDevice; }
    
private:
    VkDevice m_device;
    const VkPhysicalDeviceWrap& m_physicalDevice;
};
