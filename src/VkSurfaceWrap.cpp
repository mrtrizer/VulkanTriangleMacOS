#include "VkSurfaceWrap.hpp"

#include <vulkan/vulkan_macos.h>

#include "VkInstanceWrap.hpp"

VkSurfaceWrap::VkSurfaceWrap(const void* caMetalLayer, const VkInstanceWrap& instance)
: m_instance(instance)
{
    VkMacOSSurfaceCreateInfoMVK createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.pView = caMetalLayer;
    
    if (vkCreateMacOSSurfaceMVK(instance.instance(), &createInfo, nullptr, &m_surface) != VK_SUCCESS)
        throw std::runtime_error("Can't create macos surface");
}

VkSurfaceWrap::~VkSurfaceWrap()
{
    vkDestroySurfaceKHR(m_instance.instance(), m_surface, nullptr);
}
