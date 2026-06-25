#include "vulkan/queue.h"
#include <stdint.h>
#include <stdlib.h>

/* Queue families */
uint32_t select_queue_family(VkPhysicalDevice physical_device) {
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count,
                                           VK_NULL_HANDLE);
  /* Get family properties */
  VkQueueFamilyProperties *queue_family_properties =
      calloc(sizeof(VkQueueFamilyProperties), queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count,
                                           queue_family_properties);
  /* Queue count */
  // uint32_t queue_count[queue_family_count];
  /* Result */
  uint32_t selected_family_index = 0;
  for (uint32_t i = 0; i < queue_family_count; i++) {
    // queue_count[i] = queue_family_properties[i].queueCount;
    if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      selected_family_index = i;
    }
  }
  return selected_family_index;
}
