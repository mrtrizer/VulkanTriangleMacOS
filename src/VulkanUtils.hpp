#pragma once

#include <vector>
#include <vulkan/vulkan.h>

std::vector<VkExtensionProperties> getVkExtensions();

std::vector<VkLayerProperties> getVkValidationLayers();

bool validationLayersAvaliable(const std::vector<VkLayerProperties>& validationLayers,
                                      const std::vector<const char*>& requiredLayerNames);
