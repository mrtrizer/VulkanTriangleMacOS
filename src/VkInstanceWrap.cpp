#include "VkInstanceWrap.hpp"

#include <exception>
#include <unordered_set>
#include <sstream>

#include "VulkanUtils.hpp"

static std::unordered_set<std::string> findMissingExtensionNames(const std::vector<VkExtensionProperties>& extensions,
                                                          const std::vector<const char*>& requiredExtensionNames) {
    std::unordered_set<std::string> missingExtensionNames(requiredExtensionNames.begin(), requiredExtensionNames.end());
    for (auto extension : extensions)
        missingExtensionNames.erase(extension.extensionName);
    return missingExtensionNames;
}

// FIXME: Do we really need throw an exception right here? May be there are better ways?s
static void ensureRequiredExtensionsPresented(const std::vector<VkExtensionProperties>& extensions,
                                       const std::vector<const char*>& requiredExtensionNames) {
    auto missingExtensions = findMissingExtensionNames(extensions, requiredExtensionNames);
    if (missingExtensions.size() != 0) {
        std::stringstream ss;
        ss << "Can't find extensions:  ";
        for (auto missingExtension : missingExtensions)
            ss << missingExtension << ' ';
        throw std::runtime_error(ss.str());
    }
}

VkInstanceWrap::VkInstanceWrap(const std::vector<const char*>& requiredExtensionNames,
              const std::vector<const char*>& validationLayerNames,
              const VkApplicationInfo& appInfo)
{
    auto extensions = getVkExtensions();

    ensureRequiredExtensionsPresented(extensions, requiredExtensionNames);
    
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensionNames.size());
    createInfo.ppEnabledExtensionNames = requiredExtensionNames.data();
    createInfo.enabledLayerCount = validationLayerNames.size();
    createInfo.ppEnabledLayerNames = validationLayerNames.data();
    
    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
        throw std::runtime_error("failed to create instance!");
    
    m_messenger = createDebugMessenger(m_instance, this);
}

VkInstanceWrap::~VkInstanceWrap() {
    vkDestroyInstance(m_instance, nullptr);
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(m_instance, m_messenger, nullptr);
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
