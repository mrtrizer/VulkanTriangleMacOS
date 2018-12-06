#include "VkInstanceWrap.hpp"

#include <exception>
#include <unordered_set>
#include <sstream>
#include <algorithm>

#include "VulkanUtils.hpp"
#include "VkPhysicalDeviceWrap.hpp"
#include "VkSurfaceWrap.hpp"

std::vector<const char*> findMissedExtensionNames(const std::vector<VkExtensionProperties>& avaliableExt,
                                                  const std::vector<const char*>& requiredExtNames)
{
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
    return missing;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    for (unsigned i = 0; i < queueFamilies.size() && !indices.isComplete(); ++i) {
        const auto& queueFamily = queueFamilies[i];
        
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;
        
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (queueFamily.queueCount > 0 && presentSupport)
            indices.presentFamily = i;
    }
    
    return indices;
}

SwapChainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    
    
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }
    
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }
    
    return details;
}

bool isDeviceSuitable(VkPhysicalDevice device, const std::vector<const char*> requiredExtensionNames) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());
    
    return findMissedExtensionNames(extensions, requiredExtensionNames).empty();
}

std::vector<VkPhysicalDevice> getVkPhysicalDevices(VkInstance instance) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    return devices;
}

VkInstance createVkInstance(const std::vector<const char*>& requiredExtNames,
                            const std::vector<const char*>& validationLayerNames,
                            const VkApplicationInfo& appInfo) {

    auto missing = findMissedExtensionNames(getVkExtensions(), requiredExtNames);

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

VkPhysicalDeviceWrap VkInstanceWrap::findCompatibleDevice(const VkSurfaceWrap& surface,
                                                          const std::vector<const char*>& requiredExtensions) const
{
    auto physicalDevices = getVkPhysicalDevices(m_instance);
    
    for (const auto& physicalDevice : physicalDevices) {
        auto queueFamilies = findQueueFamilies(physicalDevice, surface.surface());
        auto swapchainSupport = querySwapchainSupport(physicalDevice, surface.surface());
        if (isDeviceSuitable(physicalDevice, requiredExtensions)
            && queueFamilies.isComplete()
            && swapchainSupport.isComplete()) {
            return VkPhysicalDeviceWrap(physicalDevice, std::move(queueFamilies), std::move(swapchainSupport));
        }
    }
    throw std::runtime_error("Failed to find a suitable GPU!");
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
