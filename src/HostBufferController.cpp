#include "HostBufferController.hpp"

#include <vulkan/vulkan.h>

#include "VkBufferWrap.hpp"
#include "VkDeviceWrap.hpp"

HostBufferController::HostBufferController(const std::shared_ptr<VkBufferWrap>& bufferWrap)
    : m_bufferWrap(bufferWrap)
{
    if (vkMapMemory(bufferWrap->deviceWrap().device(), bufferWrap->deviceMemory(), 0, bufferWrap->size(), 0, &m_mappedMemory) != VK_SUCCESS)
        throw std::runtime_error("Unable to map memory!");
}

HostBufferController::~HostBufferController() {
    vkUnmapMemory(m_bufferWrap->deviceWrap().device(), m_bufferWrap->deviceMemory());
}

void HostBufferController::copyToMemory(const void* source, size_t size) {
    if (size > m_bufferWrap->size())
        throw std::runtime_error("Data doesn't fit into buffer!");
    
    memcpy(m_mappedMemory, source, size);
}
