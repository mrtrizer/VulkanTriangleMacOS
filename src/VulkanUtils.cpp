#include "VulkanUtils.hpp"

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

// FIXME: Find some way to print list of missed layers
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
