#ifndef DONGLE_VULKAN_QUEUE
#define DONGLE_VULKAN_QUEUE

#include <vulkan/vulkan_core.h>

uint32_t get_queue_family_count(VkPhysicalDevice physical_device);
uint32_t select_queue_family(VkPhysicalDevice physical_device, uint32_t queue_family_count);

#endif
