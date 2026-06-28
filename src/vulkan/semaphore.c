#include "vulkan/semaphore.h"
#include "debug.h"

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
