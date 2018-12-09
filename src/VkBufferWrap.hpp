#pragma once

#include <vulkan/vulkan.h>

class VkDeviceWrap;
class VkPhysicalDeviceWrap;

class VkBufferWrap {
    friend class VkDeviceWrap; // for construction
public:
    ~VkBufferWrap();

    void copyToMemory(const void* source, size_t size);
    VkBuffer buffer() { return m_buffer; }
    size_t size() { return m_size; }
    
private:
    VkBufferWrap(VkDevice device, int size, const VkPhysicalDeviceWrap& physicalDevice);

    VkBuffer m_buffer;
    VkDevice m_device;
    VkDeviceMemory m_deviceMemory;
    size_t m_size;
};
