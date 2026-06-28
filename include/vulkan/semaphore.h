#ifndef DONGLE_VULKAN_SEMAPHORE
#define DONGLE_VULKAN_SEMAPHORE

#include <vulkan/vulkan_core.h>

VkSemaphore create_semaphore(VkDevice logical_device);
VkFence create_fence(VkDevice logical_device);

void destroy_semaphore(VkDevice logical_device, VkSemaphore semaphore);
void destroy_fence(VkDevice logical_device, VkFence fence);

#endif
