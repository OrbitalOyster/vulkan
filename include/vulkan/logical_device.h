#ifndef DONGLE_VULKAN_LOGICAL_DEVICE
#define DONGLE_VULKAN_LOGICAL_DEVICE

#include <vulkan/vulkan_core.h>

VkDevice create_logical_device(VkPhysicalDevice physical_device, uint32_t selected_queue_family_index);

#endif
