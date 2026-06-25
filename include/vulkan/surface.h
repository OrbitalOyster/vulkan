#ifndef DONGLE_VULKAN_SURFACE
#define DONGLE_VULKAN_SURFACE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow *glfw_window,
                            VkPhysicalDevice physical_device,
                            uint32_t selected_queue_family_index);

#endif
