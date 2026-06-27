#ifndef DONGLE_VULKAN_SURFACE
#define DONGLE_VULKAN_SURFACE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow *glfw_window,
                            VkPhysicalDevice physical_device,
                            uint32_t selected_queue_family_index);
VkSurfaceCapabilitiesKHR
get_surface_capabilities(VkPhysicalDevice physical_device,
                         VkSurfaceKHR surface);
uint32_t get_swapchain_image_count(VkSurfaceCapabilitiesKHR capabilities);
VkSurfaceFormatKHR get_surface_format(VkPhysicalDevice physical_device,
                                      VkSurfaceKHR surface);

#endif
