#include "vulkan/command_buffer.h"
#include "debug.h"

VkCommandPool create_command_pool(uint32_t selected_queue_family_index,
                                  VkDevice logical_device) {
  VkCommandPoolCreateInfo create_pool_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = selected_queue_family_index,
  };
  VkCommandPool command_pool = VK_NULL_HANDLE;
  if (vkCreateCommandPool(logical_device, &create_pool_info, VK_NULL_HANDLE,
                          &command_pool) != VK_SUCCESS)
    PANIC(1, "Unable to create command pool")
  return command_pool;
}

VkCommandBuffer create_command_buffer(VkCommandPool command_pool,
                                      VkDevice logical_device) {
  /* Command buffer */
  VkCommandBufferAllocateInfo command_buffer_allocate_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = command_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };
  VkCommandBuffer command_buffer = VK_NULL_HANDLE;
  if (vkAllocateCommandBuffers(logical_device, &command_buffer_allocate_info,
                               &command_buffer) != VK_SUCCESS)
    PANIC(1, "Unable to allocate command buffers")

  return command_buffer;
}
