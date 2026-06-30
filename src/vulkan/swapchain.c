#include "vulkan/swapchain.h"
#include "debug.h"
#include "vulkan/physical_device.h"
#include "vulkan/surface.h"
#include <stdint.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

VkExtent2D create_swapchain_extent(GLFWwindow *glfw_window,
                                   VkSurfaceCapabilitiesKHR capabilities) {
  int w, h;
  glfwGetFramebufferSize(glfw_window, &w, &h);
  uint32_t framebuffer_width = w, framebuffer_height = h;
  /* TODO: Wayland will return 0xFFFFFFFF on both */
  if (capabilities.currentExtent.width == framebuffer_width ||
      capabilities.currentExtent.height == framebuffer_height)
    return capabilities.currentExtent;
  /* Resize if necessary */
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
  return extent;
}

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
  if (vkCreateSwapchainKHR(logical_device, &create_info, VK_NULL_HANDLE,
                           &swapchain) != VK_SUCCESS)
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

struct swapchain_bundle *
create_swapchain_bundle(VkPhysicalDevice physical_device,
                        VkInstance vulkan_instance,
                        uint32_t selected_queue_family_index,
                        GLFWwindow *window, VkDevice logical_device) {
  VkSurfaceKHR surface = create_surface(
      vulkan_instance, window, physical_device, selected_queue_family_index);
  VkSurfaceCapabilitiesKHR capabilities =
      get_surface_capabilities(physical_device, surface);
  uint32_t image_count = get_swapchain_image_count(capabilities);
  VkExtent2D extent = create_swapchain_extent(window, capabilities);
  VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = extent.width,
      .height = extent.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  VkRect2D scissor = {
      .offset = {0, 0},
      .extent = extent,
  };
  VkSurfaceFormatKHR surface_format =
      get_surface_format(physical_device, surface);
  VkPresentModeKHR present_mode = get_present_mode(physical_device, surface);
  VkSwapchainKHR swapchain =
      create_swapchain(surface, image_count, surface_format, extent,
                       capabilities, present_mode, logical_device);
  VkImageView *image_views = create_swapchain_image_views(
      logical_device, swapchain, &image_count, surface_format);
  /* Done */
  struct swapchain_bundle *result = calloc(sizeof(struct swapchain_bundle), 1);
  result->window = window;
  result->surface = surface;
  result->surface_format = surface_format;
  result->present_mode = present_mode;
  result->capabilities = capabilities;
  result->extent = extent;
  result->viewport = viewport;
  result->scissor = scissor;
  result->image_count = image_count;
  result->image_views = image_views;
  result->swapchain = swapchain;
  return result;
}

void destroy_swapchain_bundle(VkDevice logical_device,
                              struct swapchain_bundle *bundle) {
  for (uint32_t i = 0; i < bundle->image_count; i++)
    vkDestroyImageView(logical_device, bundle->image_views[i], VK_NULL_HANDLE);
  vkDestroySwapchainKHR(logical_device, bundle->swapchain, VK_NULL_HANDLE);
}

void recreate_swapchain_bundle(struct swapchain_bundle *bundle,
                               VkDevice logical_device) {
  vkDeviceWaitIdle(logical_device);

  /* Destroy existing */
  vkDestroySwapchainKHR(logical_device, bundle->swapchain, VK_NULL_HANDLE);
  for (uint32_t i = 0; i < bundle->image_count; i++)
    vkDestroyImageView(logical_device, bundle->image_views[i], VK_NULL_HANDLE);

  /* Rebuild */
  bundle->extent =
      create_swapchain_extent(bundle->window, bundle->capabilities);

  bundle->viewport.width = bundle->extent.width;
  bundle->viewport.height = bundle->extent.height;
  bundle->scissor.extent = bundle->extent;

  bundle->swapchain = create_swapchain(bundle->surface, bundle->image_count,
                                       bundle->surface_format, bundle->extent,
                                       bundle->capabilities,
                                       bundle->present_mode, logical_device);

  bundle->image_views = create_swapchain_image_views(
      logical_device, bundle->swapchain, &bundle->image_count,
      bundle->surface_format);
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
                       VkImageView *image_views, uint32_t image_count) {
  vkDestroySwapchainKHR(logical_device, swapchain, VK_NULL_HANDLE);
  for (uint32_t i = 0; i < image_count; i++)
    vkDestroyImageView(logical_device, image_views[i], VK_NULL_HANDLE);
}
