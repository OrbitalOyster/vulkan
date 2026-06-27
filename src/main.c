#include "debug.h"
#include "glfw.h"
#include "vulkan/command_buffer.h"
#include "vulkan/instance.h"
#include "vulkan/logical_device.h"
#include "vulkan/physical_device.h"
#include "vulkan/pipeline.h"
#include "vulkan/queue.h"
#include "vulkan/shaders.h"
#include "vulkan/surface.h"
#include "vulkan/swapchain.h"
#include "vulkan/validation_layers.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#define MAX_FRAMES_IN_FLIGHT 2

int main(void) {
  init_glfw();
  INFO("Initialized GLFW")
  uint32_t glfw_extension_count = get_glfw_extension_count();
  INFOF("GLFW extensions: %i", glfw_extension_count)
  const char **glfw_extension_names =
      get_glfw_extension_names(&glfw_extension_count);
  for (uint32_t i = 0; i < glfw_extension_count; i++)
    INFOF("\t%s", glfw_extension_names[i])
  enable_vulkan_validation_layers();
  const char **required_validation_layers = get_required_validation_layers();
  INFO("Enabled validation layers")
  VkInstance vulkan_instance = create_vulkan_instance(
      glfw_extension_count, glfw_extension_names, required_validation_layers);
  INFO("Created Vulkan instance")
  VkPhysicalDevice physical_device = select_physical_device(vulkan_instance);
  INFO("Selected physical device");
  uint32_t selected_queue_family_index = select_queue_family(physical_device);
  VkDevice logical_device =
      create_logical_device(physical_device, selected_queue_family_index);
  INFO("Created logical device");
  GLFWwindow *glfw_window = create_window();
  INFO("Created window")
  uint32_t framebuffer_width = 0, framebuffer_height = 0;
  get_framebuffer_size(glfw_window, &framebuffer_width, &framebuffer_height);
  INFOF("GLFW framebuffer size: %u %u", framebuffer_width, framebuffer_height);
  VkSurfaceKHR surface =
      create_surface(vulkan_instance, glfw_window, physical_device,
                     selected_queue_family_index);
  INFO("Created vulkan surface");
  VkSurfaceCapabilitiesKHR capabilities =
      get_surface_capabilities(physical_device, surface);
  INFOF("Surface capabilities: %u %u", capabilities.currentExtent.width,
        capabilities.currentExtent.height)
  VkExtent2D swapchain_extent = create_swapchain_extent(
      capabilities, framebuffer_width, framebuffer_height);
  INFOF("Swapchain extent: %ux%u", swapchain_extent.width,
        swapchain_extent.height);
  uint32_t image_count = get_swapchain_image_count(capabilities);
  INFOF("Swapchain image count: %u", image_count);
  VkSurfaceFormatKHR surface_format =
      get_surface_format(physical_device, surface);
  VkPresentModeKHR present_mode = get_present_mode(physical_device, surface);
  VkSwapchainKHR swapchain =
      create_swapchain(surface, image_count, surface_format, swapchain_extent,
                       capabilities, present_mode, logical_device);
  INFO("Created swapchain")
  /* TODO: Manage separate queues edge case */
  VkShaderModule vertex_shader_module =
      create_shader_module("shaders/vert.spv", logical_device);
  VkShaderModule fragment_shader_module =
      create_shader_module("shaders/frag.spv", logical_device);
  VkPipelineShaderStageCreateInfo *shader_stages =
      create_shader_stages(vertex_shader_module, fragment_shader_module);
  INFO("Loaded shaders")
  VkRenderPass render_pass = create_render_pass(surface_format, logical_device);
  INFO("Created render pass")
  VkImageView *image_views = create_swapchain_image_views(
      logical_device, swapchain, &image_count, surface_format);
  INFO("Created swapchain image views")
  VkFramebuffer *swapchain_framebuffers = create_swapchain_framebuffers(
      image_count, image_views, render_pass, swapchain_extent, logical_device);
  INFO("Created framebuffers")

  VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = swapchain_extent.width,
      .height = swapchain_extent.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };

  VkRect2D scissor = {
      .offset = {0, 0},
      .extent = swapchain_extent,
  };

  VkPipelineLayout pipeline_layout = create_pipeline_layout(logical_device);

  VkPipeline pipeline =
      create_pipeline(&viewport, &scissor, logical_device, shader_stages,
                      pipeline_layout, render_pass);
  INFO("Created pipeline")

  VkCommandPool command_pool =
      create_command_pool(selected_queue_family_index, logical_device);
  VkCommandBuffer command_buffer =
      create_command_buffer(command_pool, logical_device);
  INFO("Created command buffer")

  VkClearValue clear_color = {{{0.1f, 0.2f, 0.3f, 1.0f}}};

  /* Semaphores */
  VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };

  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;
  VkFence inFlightFence;

  if (vkCreateSemaphore(logical_device, &semaphoreInfo, VK_NULL_HANDLE,
                        &imageAvailableSemaphore) == VK_SUCCESS &&
      vkCreateSemaphore(logical_device, &semaphoreInfo, NULL,
                        &renderFinishedSemaphore) == VK_SUCCESS &&
      vkCreateFence(logical_device, &fenceInfo, VK_NULL_HANDLE,
                    &inFlightFence) == VK_SUCCESS) {
    INFO("Created semaphores")
  } else
    PANIC(1, "Failed to create semaphores")

  /* Graphics queue */
  VkQueue graphics_queue;
  vkGetDeviceQueue(logical_device, selected_queue_family_index, 0,
                   &graphics_queue);

  /* Present queue */
  VkQueue present_queue;
  vkGetDeviceQueue(logical_device, selected_queue_family_index, 0,
                   &present_queue);

  while (!glfwWindowShouldClose(glfw_window)) {
    glfwPollEvents();

    /* Draw frame */
    vkWaitForFences(logical_device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(logical_device, 1, &inFlightFence);
    uint32_t imageIndex;
    vkAcquireNextImageKHR(logical_device, swapchain, UINT64_MAX,
                          imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(command_buffer, 0);

    // recordCommandBuffer

    VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = VK_NULL_HANDLE,
    };
    if (vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) !=
        VK_SUCCESS)
      PANIC(1, "Unable to start recording command buffer")

    VkRenderPassBeginInfo render_pass_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_pass,
        .framebuffer = swapchain_framebuffers[imageIndex],
        .renderArea.offset = {0, 0},
        .renderArea.extent = swapchain_extent,
        .clearValueCount = 1,
        .pClearValues = &clear_color,
    };
    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline);
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdDraw(command_buffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(command_buffer);
    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
      PANIC(1, "Failed to record command buffer")
    // ----------------------------------------------

    /* Submit command buffer */
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    VkSubmitInfo submit_command_buffer_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };
    if (vkQueueSubmit(graphics_queue, 1, &submit_command_buffer_info,
                      inFlightFence) != VK_SUCCESS)
      PANIC(1, "Failed to submit draw command buffer")

    /* Present */
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex,
        .pResults = VK_NULL_HANDLE,
    };
    /* Ta-da */
    vkQueuePresentKHR(present_queue, &present_info);
  }

  vkDeviceWaitIdle(logical_device);

  INFO("Starting cleanup")
  vkDestroySemaphore(logical_device, imageAvailableSemaphore, VK_NULL_HANDLE);
  vkDestroySemaphore(logical_device, renderFinishedSemaphore, VK_NULL_HANDLE);
  destroy_command_pool(logical_device, command_pool);
  vkDestroyFence(logical_device, inFlightFence, VK_NULL_HANDLE);
  for (uint32_t i = 0; i < image_count; i++)
    destroy_swapchain_buffer(logical_device, swapchain_framebuffers[i]);
  for (size_t i = 0; i < image_count; i++)
    destroy_swapchain_image_views(logical_device, image_views[i]);
  destroy_pipeline(logical_device, pipeline);
  destroy_render_pass(logical_device, render_pass);
  destroy_pipeline_layout(logical_device, pipeline_layout);
  destroy_shader_module(logical_device, vertex_shader_module);
  destroy_shader_module(logical_device, fragment_shader_module);
  destroy_swapchain(logical_device, swapchain);
  destroy_logical_device(logical_device);
  INFO("Cleanup complete")

  return EXIT_SUCCESS;
}
