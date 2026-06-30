#include "debug.h"
#include "glfw.h"
#include "vulkan/command_buffer.h"
#include "vulkan/instance.h"
#include "vulkan/logical_device.h"
#include "vulkan/physical_device.h"
#include "vulkan/pipeline.h"
#include "vulkan/queue.h"
#include "vulkan/semaphore.h"
#include "vulkan/shaders.h"
#include "vulkan/swapchain.h"
#include "vulkan/validation_layers.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

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

  struct swapchain_bundle *bundle = create_swapchain_bundle(
      physical_device, vulkan_instance, selected_queue_family_index,
      glfw_window, logical_device);
  INFO("Created swapchain")

  uint32_t image_count = bundle->image_count;

  VkRenderPass render_pass =
      create_render_pass(bundle->surface_format, logical_device);
  INFO("Created render pass")

  create_swapchain_bundle_framebuffers(bundle, logical_device, render_pass);

  /* TODO: Manage separate queues edge case */

  VkShaderModule vertex_shader_module =
      create_shader_module("shaders/vert.spv", logical_device);
  VkShaderModule fragment_shader_module =
      create_shader_module("shaders/frag.spv", logical_device);
  VkPipelineShaderStageCreateInfo *shader_stages =
      create_shader_stages(vertex_shader_module, fragment_shader_module);
  INFO("Loaded shaders")

  VkPipelineLayout pipeline_layout = create_pipeline_layout(logical_device);
  VkPipeline pipeline =
      create_pipeline(&bundle->viewport, &bundle->scissor, logical_device,
                      shader_stages, pipeline_layout, render_pass);
  INFO("Created pipeline")

  VkCommandPool command_pool =
      create_command_pool(selected_queue_family_index, logical_device);

  /* Graphics queue */
  VkQueue graphics_queue;
  vkGetDeviceQueue(logical_device, selected_queue_family_index, 0,
                   &graphics_queue);

  /* Present queue */
  VkQueue present_queue;
  vkGetDeviceQueue(logical_device, selected_queue_family_index, 0,
                   &present_queue);

  VkCommandBuffer *command_buffers =
      calloc(sizeof(VkCommandBuffer), image_count);
  VkSemaphore *image_available_semaphores =
      calloc(sizeof(VkSemaphore), image_count);
  VkSemaphore *render_finished_semaphores =
      calloc(sizeof(VkSemaphore), image_count);
  VkFence *fences = calloc(sizeof(VkFence), image_count);

  /* Create sync objects */
  for (uint32_t i = 0; i < image_count; i++) {
    command_buffers[i] = create_command_buffer(command_pool, logical_device);
    image_available_semaphores[i] = create_semaphore(logical_device);
    render_finished_semaphores[i] = create_semaphore(logical_device);
    fences[i] = create_fence(logical_device);
  }

  uint32_t current_frame = 0;
  uint32_t frame_to_render = 0;

  while (!glfwWindowShouldClose(glfw_window)) {
    glfwPollEvents();

    /* Draw frame */
    frame_to_render = current_frame % image_count;

    vkWaitForFences(logical_device, 1, &fences[frame_to_render], VK_TRUE,
                    UINT64_MAX);
    vkResetFences(logical_device, 1, &fences[frame_to_render]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(logical_device, bundle->swapchain, UINT64_MAX,
                          image_available_semaphores[frame_to_render],
                          VK_NULL_HANDLE, &imageIndex);

    begin_command_buffer(command_buffers[frame_to_render]);
    begin_render_pass(render_pass, bundle->framebuffers[imageIndex],
                      bundle->extent, command_buffers[frame_to_render],
                      pipeline, &bundle->viewport, &bundle->scissor);

    vkCmdDraw(command_buffers[frame_to_render], 3, 1, 0, 0);

    end_render_pass(command_buffers[frame_to_render]);
    end_command_buffer(command_buffers[frame_to_render]);

    /* Submit command buffer */
    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit_command_buffer_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &image_available_semaphores[frame_to_render],
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffers[frame_to_render],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &render_finished_semaphores[frame_to_render],
    };
    if (vkQueueSubmit(graphics_queue, 1, &submit_command_buffer_info,
                      fences[frame_to_render]) != VK_SUCCESS)
      PANIC(1, "Failed to submit draw command buffer")

    /* Present */
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &render_finished_semaphores[frame_to_render],
        .swapchainCount = 1,
        .pSwapchains = &bundle->swapchain,
        .pImageIndices = &imageIndex,
        .pResults = VK_NULL_HANDLE,
    };
    /* Ta-da */
    vkQueuePresentKHR(present_queue, &present_info);

    if (glfwGetKey(glfw_window, GLFW_KEY_R) == GLFW_PRESS) {
      recreate_swapchain_bundle(bundle, logical_device, render_pass);
    }

    current_frame++;
  }

  vkDeviceWaitIdle(logical_device);

  INFO("Starting cleanup")
  destroy_command_pool(logical_device, command_pool);
  for (uint32_t i = 0; i < image_count; i++) {
    destroy_semaphore(logical_device, image_available_semaphores[i]);
    destroy_semaphore(logical_device, render_finished_semaphores[i]);
    destroy_fence(logical_device, fences[i]);
  }
  destroy_pipeline(logical_device, pipeline);
  destroy_render_pass(logical_device, render_pass);
  destroy_pipeline_layout(logical_device, pipeline_layout);
  destroy_shader_module(logical_device, vertex_shader_module);
  destroy_shader_module(logical_device, fragment_shader_module);
  destroy_swapchain_bundle(logical_device, bundle);
  destroy_logical_device(logical_device);
  INFO("Cleanup complete")

  return EXIT_SUCCESS;
}
