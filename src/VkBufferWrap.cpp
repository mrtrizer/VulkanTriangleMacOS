#include "VkBufferWrap.hpp"

#include "VkDeviceWrap.hpp"

VkBufferWrap::VkBufferWrap(VkDevice device, int size, const VkPhysicalDeviceWrap& physicalDevice)
    : m_device(device)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    m_size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkBuffer buffer;
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to create vertex buffer!");
    
    m_buffer = buffer;
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = physicalDevice.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    VkDeviceMemory deviceMemory;
    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &deviceMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    m_deviceMemory = deviceMemory;
    
    if (vkBindBufferMemory(m_device, m_buffer, deviceMemory, 0) != VK_SUCCESS)
        throw std::runtime_error("Failed to bind vertex buffer!");


}

VkBufferWrap::~VkBufferWrap() {
    
    vkDestroyBuffer(m_device, m_buffer, nullptr);
    vkFreeMemory(m_device, m_deviceMemory, nullptr);
}

void VkBufferWrap::copyToMemory(const void* source, size_t size) {
    if (size > m_size)
        throw std::runtime_error("Data doesn't fit into buffer!");
    void* mappedMemory;
    if (vkMapMemory(m_device, m_deviceMemory, 0, size, 0, &mappedMemory) != VK_SUCCESS)
        throw std::runtime_error("Unable to map memory!");
    memcpy(mappedMemory, source, size);
    vkUnmapMemory(m_device, m_deviceMemory);
}
