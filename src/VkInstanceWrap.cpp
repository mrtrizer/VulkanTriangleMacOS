#include "VkInstanceWrap.hpp"

#include <exception>
#include <unordered_set>
#include <sstream>
#include <algorithm>

#include "VulkanUtils.hpp"

VkInstance createVkInstance(const std::vector<const char*>& requiredExtNames,
                            const std::vector<const char*>& validationLayerNames,
                            const VkApplicationInfo& appInfo) {
    
    auto avaliableExt = getVkExtensions();
    
    std::vector<const char*> avaliableExtNames(avaliableExt.size());
    std::transform(avaliableExt.begin(),
                   avaliableExt.end(),
                   avaliableExtNames.begin(),
                   [](const auto& e) {
                        return e.extensionName;
                    } );
    
    std::vector<const char*> missing;
    
    std::set_difference(requiredExtNames.begin(),
                        requiredExtNames.end(),
                        avaliableExtNames.begin(),
                        avaliableExtNames.end(),
                        std::back_inserter(missing),
                        [](const char* required, const char* avaliable){
                            return std::strcmp(required, avaliable) == 0;
                        });
    
    if (!missing.empty()) {
        std::ostringstream ss;
        std::copy(missing.begin(), missing.end(), std::ostream_iterator<const char*>(ss, " "));
        throw std::runtime_error("Can't find extensions:  " + ss.str());
    }
    
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtNames.size());
    createInfo.ppEnabledExtensionNames = requiredExtNames.data();
    createInfo.enabledLayerCount = validationLayerNames.size();
    createInfo.ppEnabledLayerNames = validationLayerNames.data();
    
    VkInstance instance;
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("failed to create instance!");
    return instance;
}

VkInstanceWrap::VkInstanceWrap(const std::vector<const char*>& requiredExtensionNames,
              const std::vector<const char*>& requiredLayerNames,
              const VkApplicationInfo& appInfo)
{
    auto avaliableLayers = getVkValidationLayers();
    
    std::vector<const char*> avaliableLayerNames(avaliableLayers.size());
    std::transform(avaliableLayers.begin(),
                   avaliableLayers.end(),
                   avaliableLayerNames.begin(),
                   [](const auto& e) {
                       return e.layerName;
                   } );
    
    std::vector<const char*> layerNames;
    std::set_intersection(requiredLayerNames.begin(),
                          requiredLayerNames.end(),
                          avaliableLayerNames.begin(),
                          avaliableLayerNames.end(),
                          std::back_inserter(layerNames),
                          [](const char* required, const char* avaliable) {
                              return std::strcmp(required, avaliable) == 0;
                          });
    
    auto instance = createVkInstance(requiredExtensionNames, layerNames, appInfo);
    m_instance = instance;

    if (!layerNames.empty())
        m_messenger = createDebugMessenger(instance, this);
}

VkInstanceWrap::~VkInstanceWrap() {
    vkDestroyInstance(m_instance, nullptr);
    if (m_messenger != nullptr) {
        auto destroyFunc = vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
        if (destroyFunc != nullptr)
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(destroyFunc)(m_instance, m_messenger, nullptr);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VkInstanceWrap::debugCallbackWrap(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                 void* pUserData)
{
    const auto& instanceWrap = *static_cast<VkInstanceWrap*>(pUserData);
    if (instanceWrap.m_callback != nullptr)
        instanceWrap.m_callback(messageSeverity, messageType, *pCallbackData);
    return VK_FALSE;
}

VkDebugUtilsMessengerEXT VkInstanceWrap::createDebugMessenger(VkInstance instance, VkInstanceWrap* instanceWrap) {
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = VkInstanceWrap::debugCallbackWrap;
    createInfo.pUserData = instanceWrap;
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func == nullptr)
        throw std::runtime_error("Can't find CreateDebugUtilsMessengerEXT function");
    VkDebugUtilsMessengerEXT messenger;
    if (func(instance, &createInfo, nullptr, &messenger) != VK_SUCCESS)
        throw std::runtime_error("Can't create VkDebugUtilsMessengerEXT");
    return messenger;
}
