#ifndef DONGLE_VULKAN_SWAPCHAIN
#define DONGLE_VULKAN_SWAPCHAIN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

struct swapchain_bundle {
  GLFWwindow *window;
  VkSurfaceKHR surface;
  VkSurfaceFormatKHR surface_format;
  VkPresentModeKHR present_mode;
  VkSurfaceCapabilitiesKHR capabilities;
  uint32_t image_count;

  VkExtent2D extent;
  VkViewport viewport;
  VkRect2D scissor;

  VkSwapchainKHR swapchain;
  VkImageView *image_views;
};

/* Constructors */

struct swapchain_bundle *
create_swapchain_bundle(VkPhysicalDevice physical_device,
                        VkInstance vulkan_instance,
                        uint32_t selected_queue_family_index,
                        GLFWwindow *window, VkDevice logical_device);

void recreate_swapchain_bundle(struct swapchain_bundle *bundle,
                               VkDevice logical_device);

VkExtent2D create_swapchain_extent(GLFWwindow *glfw_window,
                                   VkSurfaceCapabilitiesKHR capabilities);

VkSwapchainKHR create_swapchain(VkSurfaceKHR surface, uint32_t image_count,
                                VkSurfaceFormatKHR surface_format,
                                VkExtent2D swapchain_extent,
                                VkSurfaceCapabilitiesKHR capabilities,
                                VkPresentModeKHR present_mode,
                                VkDevice logical_device);
VkImageView *create_swapchain_image_views(VkDevice logical_device,
                                          VkSwapchainKHR swapchain,
                                          uint32_t *image_count,
                                          VkSurfaceFormatKHR surface_format);
VkFramebuffer *create_swapchain_framebuffers(uint32_t count,
                                             VkImageView *image_views,
                                             VkRenderPass renderPass,
                                             VkExtent2D swapchain_extent,
                                             VkDevice logical_device);

/* Destructors */

void destroy_swapchain_bundle(VkDevice logical_device,
                              struct swapchain_bundle *bundle);

void destroy_swapchain(VkDevice logical_device, VkSwapchainKHR swapchain,
                       VkImageView *image_views, uint32_t image_count);

#endif
