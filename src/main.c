#include "debug.h"
#include "glfw.h"
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
  INFOF("Created %u framebuffers", image_count)

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

  VkPipeline graphics_pipeline = create_graphics_pipeline(
      &viewport, &scissor, logical_device, shader_stages, render_pass);
  INFO("Created graphics pipeline")

  /* Command pools */
  VkCommandPool commandPool;
  VkCommandPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = selected_queue_family_index,
  };

  if (vkCreateCommandPool(logical_device, &poolInfo, NULL, &commandPool) ==
      VK_SUCCESS) {
    INFO("Created command pool")
  } else
    PANIC(1, "Unable to create command pool")

  /* Command buffer */

  /* Allocate */
  VkCommandBuffer commandBuffer;
  VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };
  if (vkAllocateCommandBuffers(logical_device, &allocInfo, &commandBuffer) ==
      VK_SUCCESS) {
    INFO("Allocated command buffers")
  } else
    PANIC(1, "Unable to allocate command buffers");

  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

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

  if (vkCreateSemaphore(logical_device, &semaphoreInfo, NULL,
                        &imageAvailableSemaphore) == VK_SUCCESS &&
      vkCreateSemaphore(logical_device, &semaphoreInfo, NULL,
                        &renderFinishedSemaphore) == VK_SUCCESS &&
      vkCreateFence(logical_device, &fenceInfo, NULL, &inFlightFence) ==
          VK_SUCCESS) {
    INFO("Created semaphores")
  } else
    PANIC(1, "Failed to create semaphores")

  /* Graphics queue */
  VkQueue graphicsQueue;
  vkGetDeviceQueue(logical_device, selected_queue_family_index, 0,
                   &graphicsQueue);

  /* Present queue */
  VkQueue presentQueue;
  vkGetDeviceQueue(logical_device, selected_queue_family_index, 0,
                   &presentQueue);

  while (!glfwWindowShouldClose(glfw_window)) {
    glfwPollEvents();

    /* Draw frame */
    vkWaitForFences(logical_device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(logical_device, 1, &inFlightFence);
    uint32_t imageIndex;
    vkAcquireNextImageKHR(logical_device, swapchain, UINT64_MAX,
                          imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(commandBuffer, 0);

    // recordCommandBuffer

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = NULL,
    };
    if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) ==
        VK_SUCCESS) {
      // INFO("Command buffer start")
    } else
      PANIC(1, "Unable to start recording command buffer")

    VkRenderPassBeginInfo renderPassBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_pass,
        .framebuffer = swapchain_framebuffers[imageIndex],
        .renderArea.offset = {0, 0},
        .renderArea.extent = swapchain_extent,
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphics_pipeline);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) == VK_SUCCESS) {
      // INFO("Command buffer end")
    } else {
      PANIC(1, "Failed to record command buffer")
    }
    // ----------------------------------------------

    /* Submit command buffer */
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) !=
        VK_SUCCESS)
      PANIC(1, "Failed to submit draw command buffer")

    /* Present */
    VkSwapchainKHR swapChains[] = {swapchain};
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,
        .pResults = NULL,
    };
    /* Ta-da */
    vkQueuePresentKHR(presentQueue, &presentInfo);
  }

  vkDeviceWaitIdle(logical_device);

  INFO("Starting cleanup")
  vkDestroySemaphore(logical_device, imageAvailableSemaphore, NULL);
  vkDestroySemaphore(logical_device, renderFinishedSemaphore, NULL);
  vkDestroyCommandPool(logical_device, commandPool, NULL);
  vkDestroyFence(logical_device, inFlightFence, NULL);
  for (uint32_t i = 0; i < image_count; i++)
    vkDestroyFramebuffer(logical_device, swapchain_framebuffers[i], NULL);
  vkDestroyPipeline(logical_device, graphics_pipeline, NULL);
  // vkDestroyPipelineLayout(logical_device, pipelineLayout, NULL);
  vkDestroyRenderPass(logical_device, render_pass, NULL);
  vkDestroyShaderModule(logical_device, vertex_shader_module, NULL);
  vkDestroyShaderModule(logical_device, fragment_shader_module, NULL);
  for (size_t i = 0; i < image_count; i++)
    vkDestroyImageView(logical_device, image_views[i], NULL);
  vkDestroySwapchainKHR(logical_device, swapchain, NULL);
  vkDestroyDevice(logical_device, NULL);
  // vkDestroySurfaceKHR(vulkanInstance, vulkanSurface, NULL);
  INFO("Cleanup complete")

  return EXIT_SUCCESS;
}
