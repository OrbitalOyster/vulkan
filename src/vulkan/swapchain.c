#include "vulkan/swapchain.h"
#include "debug.h"
#include <stdint.h>

VkExtent2D create_swapchain_extent(GLFWwindow *glfw_window,
                                   VkSurfaceCapabilitiesKHR capabilities

) {

  int w, h;
  glfwGetFramebufferSize(glfw_window, &w, &h);
  uint32_t framebuffer_width = w, framebuffer_height = h;

  /* TODO: Wayland will return 0xFFFFFFFF on both */
  if (capabilities.currentExtent.width == framebuffer_width ||
      capabilities.currentExtent.height == framebuffer_height)
    return capabilities.currentExtent;
  VkExtent2D extent = {
      .width = framebuffer_width,
      .height = framebuffer_height,
  };
  extent.width = framebuffer_width > capabilities.maxImageExtent.width
                     ? capabilities.maxImageExtent.width
                     : framebuffer_width;
  extent.height = framebuffer_height > capabilities.maxImageExtent.height
                      ? capabilities.maxImageExtent.height
                      : framebuffer_height;

  INFOF("Extent: %ux%u", extent.width, extent.height)

  return extent;
}

/* Create swapchain */
VkSwapchainKHR create_swapchain(VkSurfaceKHR surface, uint32_t image_count,
                                VkSurfaceFormatKHR surface_format,
                                VkExtent2D swapchain_extent,
                                VkSurfaceCapabilitiesKHR capabilities,
                                VkPresentModeKHR present_mode,
                                VkDevice logical_device) {
  VkSwapchainCreateInfoKHR create_info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = surface,
      .minImageCount = image_count,
      .imageFormat = surface_format.format,
      .imageColorSpace = surface_format.colorSpace,
      .imageExtent = swapchain_extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .preTransform = capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = present_mode,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE,
  };
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  if (vkCreateSwapchainKHR(logical_device, &create_info, NULL, &swapchain) !=
      VK_SUCCESS)
    PANIC(1, "Unable to create swapchain")
  return swapchain;
}

VkImageView *create_swapchain_image_views(VkDevice logical_device,
                                          VkSwapchainKHR swapchain,
                                          uint32_t *image_count,
                                          VkSurfaceFormatKHR surface_format) {
  /* Swap chain images */
  VkImage *images;
  vkGetSwapchainImagesKHR(logical_device, swapchain, image_count,
                          VK_NULL_HANDLE);
  images = calloc(sizeof(VkImage), *image_count);
  vkGetSwapchainImagesKHR(logical_device, swapchain, image_count, images);
  /* Swap chain image views */
  VkImageView *image_views = calloc(sizeof(VkImageView), *image_count);
  for (size_t i = 0; i < *image_count; i++) {
    VkImageViewCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = images[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = surface_format.format,
        .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
    };
    if (vkCreateImageView(logical_device, &create_info, VK_NULL_HANDLE,
                          &image_views[i]) != VK_SUCCESS)
      PANIC(1, "Failed to create image view")
  }
  return image_views;
}

VkFramebuffer *create_swapchain_framebuffers(uint32_t count,
                                             VkImageView *image_views,
                                             VkRenderPass render_pass,
                                             VkExtent2D swapchain_extent,
                                             VkDevice logical_device) {
  VkFramebuffer *swapchain_framebuffers = calloc(sizeof(VkFramebuffer), count);
  for (size_t i = 0; i < count; i++) {
    VkImageView attachments[] = {image_views[i]};
    VkFramebufferCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = render_pass,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .width = swapchain_extent.width,
        .height = swapchain_extent.height,
        .layers = 1,
    };
    if (vkCreateFramebuffer(logical_device, &create_info, NULL,
                            &swapchain_framebuffers[i]) != VK_SUCCESS)
      PANIC(1, "Failed to create framebuffer")
  }
  return swapchain_framebuffers;
}

void destroy_swapchain(VkDevice logical_device, VkSwapchainKHR swapchain,
                       VkFramebuffer *framebuffers, VkImageView *image_views,
                       uint32_t image_count) {
  vkDestroySwapchainKHR(logical_device, swapchain, VK_NULL_HANDLE);
  for (uint32_t i = 0; i < image_count; i++) {
    vkDestroyFramebuffer(logical_device, framebuffers[i], VK_NULL_HANDLE);
    vkDestroyImageView(logical_device, image_views[i], VK_NULL_HANDLE);
  }
}
