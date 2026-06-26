#ifndef DONGLE_VULKAN_SWAPCHAIN
#define DONGLE_VULKAN_SWAPCHAIN

#include <vulkan/vulkan_core.h>

VkSwapchainKHR create_swapchain(VkSurfaceKHR surface, uint32_t image_count,
                                VkSurfaceFormatKHR surface_format,
                                VkExtent2D swapchain_extent,
                                VkSurfaceCapabilitiesKHR capabilities,
                                VkPresentModeKHR present_mode,
                                VkDevice logical_device);

#endif
