#ifndef DONGLE_VULKAN_SWAPCHAIN
#define DONGLE_VULKAN_SWAPCHAIN

#include <vulkan/vulkan_core.h>

/* Constructors */
VkExtent2D create_swapchain_extent(VkSurfaceCapabilitiesKHR capabilities,
                                   uint32_t framebuffer_width,
                                   uint32_t framebuffer_height);
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

void destroy_swapchain_buffer(VkDevice logical_device, VkFramebuffer framebuffer);
void destroy_swapchain(VkDevice logical_device, VkSwapchainKHR swapchain);

#endif
