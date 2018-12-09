#pragma once

#include <memory>

class VkBufferWrap;

class HostBufferController {
public:
    HostBufferController(const std::shared_ptr<VkBufferWrap>& bufferWrap);
    ~HostBufferController();
    
    void copyToMemory(const void* source, size_t size);
    
private:
    void* m_mappedMemory;
    std::shared_ptr<VkBufferWrap> m_bufferWrap;
};
