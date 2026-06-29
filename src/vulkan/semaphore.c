#include "vulkan/semaphore.h"
#include "debug.h"
#include <vulkan/vulkan_core.h>

VkSemaphore create_semaphore(VkDevice logical_device) {
  VkSemaphoreCreateInfo semaphore_create_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VkSemaphore semaphore;
  if (vkCreateSemaphore(logical_device, &semaphore_create_info, VK_NULL_HANDLE,
                        &semaphore) != VK_SUCCESS)
    PANIC(1, "Failed to create semaphore")

  return semaphore;
}

VkFence create_fence(VkDevice logical_device) {
  VkFenceCreateInfo fence_create_info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };
  VkFence fence;
  if (vkCreateFence(logical_device, &fence_create_info, VK_NULL_HANDLE,
                    &fence) != VK_SUCCESS)
    PANIC(1, "Failed to create fence")
  return fence;
}

void destroy_semaphore(VkDevice logical_device, VkSemaphore semaphore) {
  vkDestroySemaphore(logical_device, semaphore, VK_NULL_HANDLE);
}

void destroy_fence(VkDevice logical_device, VkFence fence) {
  vkDestroyFence(logical_device, fence, VK_NULL_HANDLE);
}
