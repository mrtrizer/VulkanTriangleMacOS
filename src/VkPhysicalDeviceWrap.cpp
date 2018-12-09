#include "VkPhysicalDeviceWrap.hpp"
#include <vector>
#include <unordered_set>


VkPhysicalDeviceWrap::VkPhysicalDeviceWrap(VkPhysicalDevice physicalDevice,
                     QueueFamilyIndices queueFamilies,
                     SwapChainSupportDetails supportDetails)
    : m_physicalDevice(physicalDevice)
    , m_queueFamilies(std::move(queueFamilies))
    , m_supportDetails(std::move(supportDetails))
{
    
}

VkPhysicalDeviceMemoryProperties VkPhysicalDeviceWrap::getMemoryProperties() const {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
    return memProperties;
}

uint32_t VkPhysicalDeviceWrap::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
    auto memProperties = getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Sutable memory type is not found");
}
