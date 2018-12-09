#include "VkBufferWrap.hpp"

#include "VkDeviceWrap.hpp"

VkBufferWrap::VkBufferWrap(const VkDeviceWrap& deviceWrap, int size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    : m_deviceWrap(deviceWrap)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    m_size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkBuffer buffer;
    if (vkCreateBuffer(deviceWrap.device(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to create vertex buffer!");
    
    m_buffer = buffer;
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_deviceWrap.device(), buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = deviceWrap.physicalDevice().findMemoryType(memRequirements.memoryTypeBits, properties);
    
    VkDeviceMemory deviceMemory;
    if (vkAllocateMemory(m_deviceWrap.device(), &allocInfo, nullptr, &deviceMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    m_deviceMemory = deviceMemory;
    
    if (vkBindBufferMemory(m_deviceWrap.device(), m_buffer, deviceMemory, 0) != VK_SUCCESS)
        throw std::runtime_error("Failed to bind vertex buffer!");
}

VkBufferWrap::~VkBufferWrap() {

    vkDestroyBuffer(m_deviceWrap.device(), m_buffer, nullptr);
    vkFreeMemory(m_deviceWrap.device(), m_deviceMemory, nullptr);
}
