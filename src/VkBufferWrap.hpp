#pragma once

#include <vulkan/vulkan.h>

class VkDeviceWrap;
class VkPhysicalDeviceWrap;

class VkBufferWrap {
    friend class VkDeviceWrap; // for construction
public:
    VkBufferWrap(const VkDeviceWrap& deviceWrap, int size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~VkBufferWrap();

    VkBuffer buffer() { return m_buffer; }
    size_t size() { return m_size; }
    const VkDeviceWrap& deviceWrap() { return m_deviceWrap; }
    VkDeviceMemory deviceMemory() { return m_deviceMemory; }
    
private:

    const VkDeviceWrap& m_deviceWrap;
    VkBuffer m_buffer;
    VkDeviceMemory m_deviceMemory;
    size_t m_size;
};
