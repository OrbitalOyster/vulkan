#include "vulkan/swapchain_extent.h"
#include "debug.h"
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

VkExtent2D  create_swapchain_extent(VkPhysicalDevice physical_device) {
    /* Swap extent */
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, vulkanSurface,
                                                &surfaceCapabilities);
      bool extent_suitable = true;
      int windowWidth, windowHeight;
      glfwGetFramebufferSize(glfw_window, &windowWidth, &windowHeight);
      INFOF("GLFW framebuffer size: %u %u", windowWidth, windowHeight)

      VkExtent2D actualExtent = {
          .width = windowWidth,
          .height = windowHeight,
      };

      INFOF("Surface capabilities: %u %u", surfaceCapabilities.currentExtent.width,
            surfaceCapabilities.currentExtent.height)
      /* TODO: Wayland will return 0xFFFFFFFF on both */
      if (surfaceCapabilities.currentExtent.width != (uint32_t)windowWidth ||
          surfaceCapabilities.currentExtent.height != (uint32_t)windowHeight) {
        INFO("Resizing swapchain extent")
        extent_suitable = false;
        actualExtent.width =
            (uint32_t)windowWidth > surfaceCapabilities.maxImageExtent.width
                ? surfaceCapabilities.maxImageExtent.width
                : windowWidth;
        actualExtent.width =
            (uint32_t)windowWidth < surfaceCapabilities.minImageExtent.width
                ? surfaceCapabilities.minImageExtent.width
                : windowWidth;
        actualExtent.height =
            (uint32_t)windowHeight > surfaceCapabilities.maxImageExtent.height
                ? surfaceCapabilities.maxImageExtent.height
                : windowHeight;
        actualExtent.height =
            (uint32_t)windowHeight < surfaceCapabilities.minImageExtent.height
                ? surfaceCapabilities.minImageExtent.height
                : windowHeight;
      }
      VkExtent2D swapchainExtent =
          extent_suitable ? surfaceCapabilities.currentExtent : actualExtent;

      INFOF("Swapchain extent: %ux%u", swapchainExtent.width,
            swapchainExtent.height)
}
