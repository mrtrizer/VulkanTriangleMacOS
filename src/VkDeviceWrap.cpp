#include "VkDeviceWrap.hpp"

#include <vulkan/vulkan.hpp>

VkDeviceWrap::VkDeviceWrap(const VkPhysicalDeviceWrap& physicalDevice,
             const VkPhysicalDeviceFeatures& deviceFeatures,
             const std::vector<const char*>& validationLayerNames,
             const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
             const std::vector<const char*>& extensionNames)
    : m_physicalDevice(physicalDevice)
{
    
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = extensionNames.size();
    createInfo.ppEnabledExtensionNames = extensionNames.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayerNames.size());
    createInfo.ppEnabledLayerNames = validationLayerNames.data();
    
    if (vkCreateDevice(physicalDevice.physicalDevice(), &createInfo, nullptr, &m_device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device!");
}

VkDeviceWrap::~VkDeviceWrap()
{
    vkDestroyDevice(m_device, nullptr);
}
