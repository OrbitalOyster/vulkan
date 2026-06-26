#include "vulkan/surface.h"
#include "debug.h"
#include <stdint.h>
#include <vulkan/vulkan_core.h>

VkSurfaceKHR create_surface(VkInstance vulkan_instance, GLFWwindow *glfw_window,
                            VkPhysicalDevice physical_device,
                            uint32_t selected_queue_family_index) {
  /* Create surface */
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  if (glfwCreateWindowSurface(vulkan_instance, glfw_window, NULL, &surface) !=
      VK_SUCCESS)
    PANIC(1, "Unable to create Vulkan surface")
  /* Verify surface support */
  VkBool32 supported = VK_FALSE;
  vkGetPhysicalDeviceSurfaceSupportKHR(
      physical_device, selected_queue_family_index, surface, &supported);
  /*
   * TODO: Edge case
   * "It's actually possible that the queue families supporting drawing
   * commands and the ones supporting presentation do not overlap"
   */
  if (supported != VK_TRUE)
    PANIC(1, "No surface support")
  return surface;
}

VkSurfaceCapabilitiesKHR
get_surface_capabilities(VkPhysicalDevice physical_device,
                         VkSurfaceKHR surface) {
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface,
                                            &capabilities);
  return capabilities;
}

uint32_t get_swapchain_image_count(VkSurfaceCapabilitiesKHR capabilities) {
  uint32_t min = capabilities.minImageCount;
  uint32_t max = capabilities.maxImageCount;
  /* At least min + 1 */
  uint32_t image_count = min + 1;
  /* capabilities.maxImageCount == 0 means unlimited */
  if (max > 0 && image_count > max)
    image_count = max;
  return image_count;
}

VkSurfaceFormatKHR get_surface_format(VkPhysicalDevice physical_device,
                                      VkSurfaceKHR surface) {
  /* Surface formats */
  uint32_t format_count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count,
                                       NULL);
  if (format_count == 0)
    PANIC(1, "No surface formats available")
  VkSurfaceFormatKHR *all_formats =
      calloc(sizeof(VkSurfaceFormatKHR), format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count,
                                       all_formats);
  /* Look for VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR */
  VkSurfaceFormatKHR format = {0};
  for (uint32_t i = 0; i < format_count; i++) {
    if (all_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
        all_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      format = all_formats[i];
      break;
    }
  }
  /* TODO: Didn't found format? */
  return format;
}

void destroy_surface(VkInstance instance, VkSurfaceKHR surface) {
  vkDestroySurfaceKHR(instance, surface, VK_NULL_HANDLE);
}
