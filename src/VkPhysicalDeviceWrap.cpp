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
