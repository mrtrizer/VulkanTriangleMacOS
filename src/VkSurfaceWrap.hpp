#pragma once

#include <vulkan/vulkan.h>

class VkInstanceWrap;

class VkSurfaceWrap {
public:
    VkSurfaceWrap(const void* caMetalLayer, const VkInstanceWrap& instance);
    ~VkSurfaceWrap();
    VkSurfaceKHR surface() const { return m_surface; }
    
private:
    VkSurfaceKHR m_surface;
    const VkInstanceWrap& m_instance;
};
