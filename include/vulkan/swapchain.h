#ifndef DONGLE_VULKAN_SWAPCHAIN
#define DONGLE_VULKAN_SWAPCHAIN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

struct swapchain_bundle {
  GLFWwindow *window;
  VkSurfaceKHR surface;
  VkPresentModeKHR present_mode;
  VkSurfaceCapabilitiesKHR capabilities;
  uint32_t image_count;
  VkExtent2D extent;
  VkViewport viewport;
  VkRect2D scissor;
  VkSwapchainKHR swapchain;
  VkImageView *image_views;
};

struct swapchain_bundle *
create_swapchain_bundle(VkPhysicalDevice physical_device,
                        VkInstance vulkan_instance,
                        uint32_t selected_queue_family_index,
                        GLFWwindow *window, VkDevice logical_device);

void recreate_swapchain_bundle(struct swapchain_bundle *bundle,
                               VkDevice logical_device);

/*
VkFramebuffer *create_swapchain_framebuffers(uint32_t count,
                                             VkImageView *image_views,
                                             VkRenderPass renderPass,
                                             VkExtent2D swapchain_extent,
                                             VkDevice logical_device);
                                             */

void destroy_swapchain_bundle(VkDevice logical_device,
                              struct swapchain_bundle *bundle);

#endif
