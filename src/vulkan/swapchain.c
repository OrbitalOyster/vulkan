#include "vulkan/swapchain.h"
#include <vulkan/vulkan_core.h>

#include "debug.h"

VkSwapchainKHR create_swapchain() {
    /* Create swapchain */
     uint32_t minSwapchainImages = surfaceCapabilities.minImageCount;
     INFOF("Minimum swapchain images: %u", minSwapchainImages);
     uint32_t maxSwapchainImages = surfaceCapabilities.maxImageCount;
     INFOF("Maximum swapchain images: %u", maxSwapchainImages);
     uint32_t imageCount = minSwapchainImages + 1;
     if (surfaceCapabilities.maxImageCount > 0 &&
         imageCount > surfaceCapabilities.maxImageCount)
       imageCount = surfaceCapabilities.maxImageCount;

     VkSwapchainCreateInfoKHR createSwapchainInfo = {
         .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
         .surface = vulkanSurface,
         .minImageCount = imageCount,
         .imageFormat = surfaceFormat.format,
         .imageColorSpace = surfaceFormat.colorSpace,
         .imageExtent = swapchainExtent,
         .imageArrayLayers = 1,
         .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
         .preTransform = surfaceCapabilities.currentTransform,
         .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
         .presentMode = presentMode,
         .clipped = VK_TRUE,
         .oldSwapchain = VK_NULL_HANDLE,
     };

     VkSwapchainKHR swapchain;
     if (vkCreateSwapchainKHR(logical_device, &createSwapchainInfo, NULL,
                              &swapchain) != VK_SUCCESS)
       PANIC(1, "Unable to create swapchain")

     return swapchain;
}
