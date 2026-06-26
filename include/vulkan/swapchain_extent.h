#ifndef DONGLE_VULKAN_SWAPCHAIN_EXTENT
#define DONGLE_VULKAN_SWAPCHAIN_EXTENT

#include <vulkan/vulkan_core.h>

VkExtent2D create_swapchain_extent(VkSurfaceCapabilitiesKHR capabilities,
                                   uint32_t framebuffer_width,
                                   uint32_t framebuffer_height);

#endif
