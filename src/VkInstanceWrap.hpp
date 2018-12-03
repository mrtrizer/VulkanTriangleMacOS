#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <functional>

class VkInstanceWrap {
    friend PFN_vkDebugUtilsMessengerCallbackEXT; // This static function needs access to m_callback
public:
    using DebugCallback = std::function<void(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT& pCallbackData)>;
    
    VkInstanceWrap(const std::vector<const char*>& requiredExtensionNames,
                  const std::vector<const char*>& validationLayerNames,
                  const VkApplicationInfo& appInfo);
    
    VkInstanceWrap(const VkInstanceWrap&) = delete;
    VkInstanceWrap& operator=(const VkInstanceWrap&) = delete;
    VkInstanceWrap(VkInstanceWrap&& from) = delete;
    VkInstanceWrap& operator=(VkInstanceWrap&& from) = delete;
    
    ~VkInstanceWrap();
    
    VkInstance instance() const { return m_instance; }
    
    void setDebugCallback(DebugCallback callback) { m_callback = callback; }
    
private:
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_messenger;
    DebugCallback m_callback;
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallbackWrap(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                             void* pUserData);
    static VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance, VkInstanceWrap* instanceWrap);
};

