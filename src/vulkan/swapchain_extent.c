#include "vulkan/swapchain_extent.h"
#include "debug.h"
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

VkExtent2D create_swapchain_extent(VkSurfaceCapabilitiesKHR capabilities,
                                   uint32_t framebuffer_width,
                                   uint32_t framebuffer_height) {
  VkExtent2D actual_extent = {
      .width = framebuffer_width,
      .height = framebuffer_height,
  };
  bool extent_suitable = true;

  /* TODO: Wayland will return 0xFFFFFFFF on both */
  if (capabilities.currentExtent.width != framebuffer_width ||
      capabilities.currentExtent.height != framebuffer_height) {
    INFO("Resizing swapchain extent")
    extent_suitable = false;
    actual_extent.width = framebuffer_width > capabilities.maxImageExtent.width
                              ? capabilities.maxImageExtent.width
                              : framebuffer_width;
    actual_extent.width = framebuffer_width < capabilities.minImageExtent.width
                              ? capabilities.minImageExtent.width
                              : framebuffer_height;
    actual_extent.height =
        framebuffer_height > capabilities.maxImageExtent.height
            ? capabilities.maxImageExtent.height
            : framebuffer_height;
    actual_extent.height =
        framebuffer_height < capabilities.minImageExtent.height
            ? capabilities.minImageExtent.height
            : framebuffer_height;
  }
  return extent_suitable ? capabilities.currentExtent : actual_extent;
}
