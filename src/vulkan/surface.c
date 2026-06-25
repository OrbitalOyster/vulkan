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

void destroy_surface(VkInstance instance, VkSurfaceKHR surface) {
  vkDestroySurfaceKHR(instance, surface, VK_NULL_HANDLE);
}
