#ifndef DONGLE_VULKAN_COMMAND_BUBBER
#define DONGLE_VULKAN_COMMAND_BUBBER

#include <vulkan/vulkan_core.h>

VkCommandPool create_command_pool(uint32_t selected_queue_family_index,
                                  VkDevice logical_device);
VkCommandBuffer create_command_buffer(VkCommandPool command_pool,
                                      VkDevice logical_device);
void destroy_command_pool(VkDevice logical_device, VkCommandPool command_pool);
#endif
