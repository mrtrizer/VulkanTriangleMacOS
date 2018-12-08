#include <vulkan/vulkan.h>
#include <vulkan/vulkan_macos.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>
#include <unordered_set>

#include "macOSInterface.hpp"
#include "VulkanUtils.hpp"
#include "VkInstanceWrap.hpp"
#include "VkSurfaceWrap.hpp"
#include "VkDeviceWrap.hpp"
#include "VkSwapchainWrap.hpp"

VkInstanceWrap createVkInstance(const std::vector<const char*>& requiredExtensionNames,
                            const std::vector<const char*>& validationLayerNames) {
    std::cout << "available extensions:" << std::endl;
    for (const auto& extension : getVkExtensions())
        std::cout << '\t' << extension.extensionName << std::endl;
    
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "FlappyEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    return VkInstanceWrap(requiredExtensionNames, validationLayerNames, appInfo);
}

VkDeviceWrap createVkLogicalDevice(const VkPhysicalDeviceWrap& physicalDevice,
                               const std::vector<const char*>& validationLayerNames,
                               const std::vector<const char*>& extensionNames) {
    // Common for all queues
    float queuePriority = 1.0f;

    // Queues
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (auto familyIndex : physicalDevice.queueFamilies().uniqueIndices()){
        VkDeviceQueueCreateInfo& queueCreateInfo = queueCreateInfos.emplace_back();
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = familyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
    }
    
    VkPhysicalDeviceFeatures features;

    return VkDeviceWrap(physicalDevice, features, validationLayerNames, queueCreateInfos, extensionNames);
}

VkQueue getVkQueue(VkDevice device, unsigned family, unsigned index) {
    VkQueue queue;
    vkGetDeviceQueue(device, family, index, &queue);
    return queue;
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

    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;
        else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            bestMode = availablePresentMode;
    }

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

uint32_t selectImageCount(const SwapChainSupportDetails& swapChainSupport) {
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
    // A value of 0 for maxImageCount means that there is no limit besides memory requirements,
    // which is why we need to check for that.
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    return imageCount;
}

std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file " + filename);
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }
    return shaderModule;
}

VkPipelineShaderStageCreateInfo prepareStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule module) {
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = stage;
    vertShaderStageInfo.module = module;
    vertShaderStageInfo.pName = "main";
    return vertShaderStageInfo;
}

VkPipelineLayout createPipelineLayout(VkDevice device) {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    return pipelineLayout;
}

VkRenderPass createRenderPass(VkDevice device, VkFormat imageFormat) {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.flags = 0;
    colorAttachment.format = imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkRenderPass renderPass;
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    return renderPass;
}

VkPipeline createGraphicsPipeline(VkDevice device,
                                  VkPipelineLayout pipelineLayout,
                                  VkRenderPass renderPass,
                                  const VkExtent2D& extent)
{

    VkShaderModule vertShaderModule = createShaderModule(device, readFile("shaders/vert.spv"));
    VkShaderModule fragShaderModule = createShaderModule(device, readFile("shaders/frag.spv"));

    VkPipelineShaderStageCreateInfo shaderStagesCreateInfo[] = {
        prepareStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule),
        prepareStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderModule)
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) extent.width;
    viewport.height = (float) extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_TRUE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStagesCreateInfo;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    VkPipeline graphicsPipeline;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);

    return graphicsPipeline;
}

std::vector<VkImageView> createSwapchainImageViews( VkDevice logicalDevice,
                                                    const std::vector<VkImage>& swapchainImages,
                                                    VkFormat format)
{
    std::vector<VkImageView> swapChainImageViews;

    for (auto image: swapchainImages) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapChainImageViews.emplace_back()) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }

    return swapChainImageViews;
}

std::vector<VkFramebuffer> createFramebuffers(VkDevice device,
                                 VkRenderPass renderPass,
                                 const std::vector<VkImageView>& imageViewes,
                                 const VkExtent2D& extent)
{
    std::vector<VkFramebuffer> framebuffers;
    framebuffers.reserve(imageViewes.size());

    for (auto imageView : imageViewes) {
        VkImageView attachments[] = { imageView };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers.emplace_back()) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

    return framebuffers;
}

VkCommandPool createCommandPool(VkDevice device, const QueueFamilyIndices& queueFamilies) {

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilies.graphicsFamily;
    poolInfo.flags = 0; // Optional

    VkCommandPool commandPool;
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
    return commandPool;
}

std::vector<VkCommandBuffer> createCommandBuffers(VkDevice device,
                                                  VkCommandPool commandPool,
                                                  VkRenderPass renderPass,
                                                  VkPipeline pipeline,
                                                  const std::vector<VkFramebuffer>& framebuffers,
                                                  const VkExtent2D& extent) {
    const size_t frameNumber = framebuffers.size();

    std::vector<VkCommandBuffer> commandBuffers(frameNumber);
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    for (int i = 0; i < frameNumber; ++i) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = extent;
        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(commandBuffers[i]);
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    return commandBuffers;
}

VkSemaphore createSemaphore(VkDevice device) {
    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphore!");
    }
    return semaphore;
}

struct UpdateInfo {
    VkDevice device;
    VkSwapchainKHR swapchain;
    std::vector<VkCommandBuffer> commandBuffers;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
};

void update(void* userDataPtr) {
    if (userDataPtr == nullptr)
        return;
    auto updateInfo = *static_cast<UpdateInfo*>(userDataPtr);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(updateInfo.device,
                          updateInfo.swapchain,
                          std::numeric_limits<uint64_t>::max(),
                          updateInfo.imageAvailableSemaphore,
                          VK_NULL_HANDLE, &imageIndex);
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {updateInfo.imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &updateInfo.commandBuffers[imageIndex];
    VkSemaphore signalSemaphores[] = {updateInfo.renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    if (vkQueueSubmit(updateInfo.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {updateInfo.swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;
    vkQueuePresentKHR(updateInfo.presentQueue, &presentInfo);
}

int main(int argc, char* argv[]) {

    const std::vector<const char*> requiredValidationLayerNames = {
        "VK_LAYER_LUNARG_standard_validation",
        "VK_LAYER_LUNARG_parameter_validation",
        "VK_LAYER_LUNARG_core_validation",
        "VK_LAYER_LUNARG_object_tracker",
        "VK_LAYER_GOOGLE_threading",
        "VK_LAYER_GOOGLE_unique_objects"
    };

    auto validationLayers = getVkValidationLayers();
    std::cout << "Validation layers:" << std::endl;
    for (auto validationLayer : validationLayers)
        std::cout << '\t' << validationLayer.layerName << std::endl;

    auto requiredInstanceExtensionNames = std::vector<const char*>{ "VK_KHR_surface", "VK_MVK_macos_surface" };
    requiredInstanceExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    std::cout << "Required extensions for instance:" << std::endl;
    for (auto requiredExtensionName : requiredInstanceExtensionNames)
        std::cout << '\t' << requiredExtensionName << std::endl;

    // Create VK instance
    auto instance = createVkInstance(requiredInstanceExtensionNames, requiredValidationLayerNames);
    
    instance.setDebugCallback([](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                 const VkDebugUtilsMessengerCallbackDataEXT& pCallbackData) {
        std::cout << pCallbackData.pMessage << std::endl;
    });
    
    auto macOsApp = createMacOsApp(update);

    VkSurfaceWrap surface(macOsApp.caMetalLayer, instance);
    
    const std::vector<const char*> deviceRequiredExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME};
    
    auto physicalDevice = instance.findCompatibleDevice(surface, deviceRequiredExtensions);
    
    std::cout << "Graphics family index: " << physicalDevice.queueFamilies().graphicsFamily << std::endl;
    std::cout << "Present family index: " << physicalDevice.queueFamilies().presentFamily << std::endl;
    
    auto logicalDevice = createVkLogicalDevice(physicalDevice, requiredValidationLayerNames, deviceRequiredExtensions);
    
    SwapchainSettings swapchainSettings {
        .surfaceFormat = chooseSwapSurfaceFormat(physicalDevice.supportDetails().formats),
        .presentMode = chooseSwapPresentMode(physicalDevice.supportDetails().presentModes),
        .extent = chooseSwapExtent(physicalDevice.supportDetails().capabilities),
        .imageCount = selectImageCount(physicalDevice.supportDetails()),
        .transform = physicalDevice.supportDetails().capabilities.currentTransform
    };
    auto swapchain = VkSwapchainWrap(logicalDevice, surface, swapchainSettings);
    
    auto swapchainImages = swapchain.getSwapchainImages();
    
    auto swapchainImageViews = createSwapchainImageViews(logicalDevice.device(), swapchainImages, swapchainSettings.surfaceFormat.format);
    
    auto pipelineLayout = createPipelineLayout(logicalDevice.device());
    
    auto renderPass = createRenderPass(logicalDevice.device(), swapchainSettings.surfaceFormat.format);
    
    auto framebuffers = createFramebuffers(logicalDevice.device(), renderPass, swapchainImageViews, swapchainSettings.extent);
    
    auto graphicsPipeline = createGraphicsPipeline(logicalDevice.device(), pipelineLayout, renderPass, swapchainSettings.extent);
    
    auto commandPool = createCommandPool(logicalDevice.device(), physicalDevice.queueFamilies());
    
    auto commandBuffers = createCommandBuffers(logicalDevice.device(), commandPool, renderPass, graphicsPipeline, framebuffers, swapchainSettings.extent);
    
    auto imageAvailableSemaphore = createSemaphore(logicalDevice.device());
    auto renderFinishedSemaphore = createSemaphore(logicalDevice.device());
    
    UpdateInfo updateInfo = UpdateInfo {
        .device = logicalDevice.device(),
        .swapchain = swapchain.swapchain(),
        .commandBuffers = commandBuffers,
        .imageAvailableSemaphore = imageAvailableSemaphore,
        .renderFinishedSemaphore = renderFinishedSemaphore,
        .graphicsQueue = getVkQueue(logicalDevice.device(), physicalDevice.queueFamilies().graphicsFamily, 0),
        .presentQueue = getVkQueue(logicalDevice.device(), physicalDevice.queueFamilies().presentFamily, 0),
    };
    
    setUserData(&macOsApp, &updateInfo);
    
    runMacOsApp(&macOsApp);
    
    setUserData(&macOsApp, nullptr);

    vkDeviceWaitIdle(logicalDevice.device());

    // Vulkan cleanup
    {
        vkDestroySemaphore(logicalDevice.device(), renderFinishedSemaphore, nullptr);
        vkDestroySemaphore(logicalDevice.device(), imageAvailableSemaphore, nullptr);
        vkDestroyCommandPool(logicalDevice.device(), commandPool, nullptr);
        for (auto framebuffer : framebuffers) {
                vkDestroyFramebuffer(logicalDevice.device(), framebuffer, nullptr);
        }
        vkDestroyRenderPass(logicalDevice.device(), renderPass, nullptr);
        vkDestroyPipeline(logicalDevice.device(), graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(logicalDevice.device(), pipelineLayout, nullptr);
        for (auto imageView : swapchainImageViews) {
            vkDestroyImageView(logicalDevice.device(), imageView, nullptr);
        }
    }

    return EXIT_SUCCESS;
}
