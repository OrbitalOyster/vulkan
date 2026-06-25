#ifndef DONGLE_VULKAN_INSTANCE
#define DONGLE_VULKAN_INSTANCE

#include <vulkan/vulkan_core.h>

VkInstance create_vulkan_instance(uint32_t glfw_extension_count,
                                  const char **glfw_extensions,
                                  const char **required_validation_layers);

#endif
