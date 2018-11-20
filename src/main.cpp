#include <SDL.h>
#include <SDL_vulkan.h>
#include <SDL_system.h>
#include <SDL_syswm.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_macos.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <list>
#include <sstream>
#include <unordered_set>

#include "macOSInterface.hpp"

std::unordered_set<std::string> findMissingExtensionNames(const std::vector<VkExtensionProperties>& extensions,
                                         const std::vector<const char*>& requiredExtensionNames) {
    std::unordered_set<std::string> missingExtensionNames(requiredExtensionNames.begin(), requiredExtensionNames.end());
    for (auto extension : extensions)
        missingExtensionNames.erase(extension.extensionName);
    return missingExtensionNames;
}

void ensureRequiredExtensionsPresented(const std::vector<VkExtensionProperties>& extensions,
                                       const std::vector<const char*>& requiredExtensionNames) {
    auto missingExtensions = findMissingExtensionNames(extensions, requiredExtensionNames);
    if (missingExtensions.size() != 0) {
        std::stringstream ss;
        ss << "Can't find extensions:  ";
        for (auto missingExtension : missingExtensions) {
            ss << missingExtension << ' ';
        }
        throw std::runtime_error(ss.str());
    }
}

std::vector<const char*> getRequiredSDLExtensionNames(SDL_Window* window) {
    unsigned requiredExtensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &requiredExtensionCount, nullptr);
    std::vector<const char*> requiredExtensionNames(requiredExtensionCount);
    SDL_Vulkan_GetInstanceExtensions(window, &requiredExtensionCount, requiredExtensionNames.data());
    return requiredExtensionNames;
}

std::vector<VkExtensionProperties> getVkExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    return extensions;
}

std::vector<VkLayerProperties> getVkValidationLayers() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    return availableLayers;
}

bool validationLayersAvaliable(const std::vector<VkLayerProperties>& validationLayers,
                               const std::vector<const char*>& requiredLayerNames) {
    for (const char* requiredLayerName : requiredLayerNames) {
        auto predicate = [requiredLayerName](const auto& item) {
            return strcmp(item.layerName, requiredLayerName) == 0;
        };
        if (std::find_if(validationLayers.begin(), validationLayers.end(), predicate) == validationLayers.end())
            return false;
    }
    return true;
}

VkInstance createVkInstance(const std::vector<const char*>& requiredExtensionNames,
                            const std::vector<const char*>& validationLayerNames) {
    auto extensions = getVkExtensions();
    std::cout << "available extensions:" << std::endl;
    for (const auto& extension : extensions)
        std::cout << '\t' << extension.extensionName << std::endl;

    ensureRequiredExtensionsPresented(extensions, requiredExtensionNames);

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "FlappyEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensionNames.size());
    createInfo.ppEnabledExtensionNames = requiredExtensionNames.data();
    createInfo.enabledLayerCount = validationLayerNames.size();
    createInfo.ppEnabledLayerNames = validationLayerNames.data();

    VkInstance instance;
    auto result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        std::cout << result << std::endl;
        throw std::runtime_error("failed to create instance!");
    }
    return instance;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

VkDebugUtilsMessengerEXT createVkDebugUtilsMessenger(VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT callback) {
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = callback;
    createInfo.pUserData = nullptr; // Optional
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func == nullptr)
        throw std::runtime_error("Can't find CreateDebugUtilsMessengerEXT function");
    VkDebugUtilsMessengerEXT debugUtilsMessenger;
    if (func(instance, &createInfo, nullptr, &debugUtilsMessenger) != VK_SUCCESS)
        throw std::runtime_error("Can't create VkDebugUtilsMessengerEXT");
    return debugUtilsMessenger;
}

void destroyVkDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, messenger, nullptr);
}

struct QueueFamilyIndices {
    unsigned graphicsFamily = UINT_MAX;
    unsigned presentFamily = UINT_MAX;

    bool isComplete() {
        return graphicsFamily != UINT_MAX && presentFamily != UINT_MAX;
    }

    std::vector<unsigned> indices() {
        return {graphicsFamily, presentFamily};
    }

    std::unordered_set<unsigned> uniqueIndices() {
        auto indicesVector = indices();
        return {indicesVector.begin(), indicesVector.end()};
    }
};

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

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
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

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*> requiredExtensionNames) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

    bool allExtensionsPresented = findMissingExtensionNames(extensions, requiredExtensionNames).size() == 0;

    bool swapChainAdequate = false;
    if (allExtensionsPresented) {
        auto swapChainSupport = querySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return findQueueFamilies(device, surface).isComplete() && allExtensionsPresented && swapChainAdequate;
}

std::vector<VkPhysicalDevice> getVkPhysicalDevices(VkInstance instance) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    return devices;
}

VkPhysicalDevice findVkPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*> requiredExtensions) {
    auto physicalDevices = getVkPhysicalDevices(instance);

    for (const auto& physicalDevice : physicalDevices) {
        if (isDeviceSuitable(physicalDevice, surface, requiredExtensions)) {
            return physicalDevice;
        }
    }
    throw std::runtime_error("Failed to find a suitable GPU!");
}

VkDevice createVkLogicalDevice(VkPhysicalDevice physicalDevice,
                               QueueFamilyIndices queueFamilies,
                               const std::vector<const char*>& validationLayerNames) {
    // Common for all queues
    float queuePriority = 1.0f;

    // Queues
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (auto familyIndex : queueFamilies.uniqueIndices()){
        VkDeviceQueueCreateInfo& queueCreateInfo = queueCreateInfos.emplace_back();
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = familyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
    }

    // Features
    VkPhysicalDeviceFeatures deviceFeatures = {};

    // Device
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayerNames.size());
    createInfo.ppEnabledLayerNames = validationLayerNames.data();

    VkDevice device;
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device!");
    return device;
}

VkQueue getVkQueue(VkDevice device, unsigned family, unsigned index) {
    VkQueue queue;
    vkGetDeviceQueue(device, family, index, &queue);
    return queue;
}

VkSurfaceKHR createVkSDLSurface(SDL_Window* window, VkInstance instance) {
    VkSurfaceKHR surface;

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(window, &info) != SDL_TRUE) {
        std::cout << SDL_GetError() << std::endl;
        throw std::runtime_error("Can't extract window id");
    }

    VkMacOSSurfaceCreateInfoMVK createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.pView = initMetal(info.info.cocoa.window);

    if (vkCreateMacOSSurfaceMVK(instance, &createInfo, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Can't create macos surface");

    return surface;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

//    for (const auto& availablePresentMode : availablePresentModes) {
//        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
//            return availablePresentMode;
//        else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
//            bestMode = availablePresentMode;
//    }

    return bestMode;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {640, 480};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

VkSwapchainKHR createSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
    // A value of 0 for maxImageCount means that there is no limit besides memory requirements,
    // which is why we need to check for that.
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    auto queueFamilyIndices = indices.indices();

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
        createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapChain;
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    return swapChain;
}

int main(int argc, char* argv[]) {

    SDL_Window *window;                    // Declare a pointer

    SDL_Init(SDL_INIT_EVERYTHING);              // Initialize SDL2

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "An SDL2 window",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        640,                               // width, in pixels
        480,                               // height, in pixels
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE                  // flags - see below
    );

    // Check that the window was successfully created
    if (window == NULL) {
        // throw std::runtime_error("Could not create window: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_LUNARG_standard_validation",
        "VK_LAYER_LUNARG_parameter_validation",
        "VK_LAYER_LUNARG_core_validation",
        "VK_LAYER_LUNARG_object_tracker",
        "VK_LAYER_GOOGLE_threading",
        "VK_LAYER_GOOGLE_unique_objects"
    };

    if (!validationLayersAvaliable(getVkValidationLayers(), validationLayers))
        throw std::runtime_error("Validation layer is not avaliable");

    auto requiredInstanceExtensionNames = getRequiredSDLExtensionNames(window);
    requiredInstanceExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    std::cout << "Required extensions for instance:" << std::endl;
    for (auto requiredExtensionName : requiredInstanceExtensionNames)
        std::cout << '\t' << requiredExtensionName << std::endl;

    // Create VK instance
    VkInstance instance = createVkInstance(requiredInstanceExtensionNames, validationLayers);

    auto debugMessenger = createVkDebugUtilsMessenger(instance, debugCallback);

    auto surface = createVkSDLSurface(window, instance);

    const std::vector<const char*> deviceRequiredExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    auto physicalDevice = findVkPhysicalDevice(instance, surface, deviceRequiredExtensions);

    auto queueFamilies = findQueueFamilies(physicalDevice, surface);

    std::cout << "Graphics family index: " << queueFamilies.graphicsFamily << std::endl;
    std::cout << "Present family index: " << queueFamilies.presentFamily << std::endl;

    auto logicalDevice = createVkLogicalDevice(physicalDevice, queueFamilies, validationLayers);

    auto queue = getVkQueue(logicalDevice, queueFamilies.graphicsFamily, 0);

    auto swapChain = createSwapChain(physicalDevice, logicalDevice, surface);

    // The window is open: could enter program loop here (see SDL_PollEvent())
    bool quit = false;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                quit = true;
        }
        /* do some other stuff here -- draw your app, etc. */
    }

    // Vulkan cleanup
    {
        destroyVkDebugUtilsMessenger(instance, debugMessenger);
        vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
        vkDestroyDevice(logicalDevice, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
    return EXIT_SUCCESS;
}
