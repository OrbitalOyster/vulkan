#ifndef DONGLE_VULKAN_SHADERS
#define DONGLE_VULKAN_SHADERS

#include <vulkan/vulkan_core.h>

VkShaderModule create_shader_module(char *filename, VkDevice logical_device);
VkPipelineShaderStageCreateInfo *
create_shader_stages(VkShaderModule vertex_shader_module,
                     VkShaderModule fragment_shader_module);
/* Desctructors */
void destroy_shader_module(VkDevice logical_device, VkShaderModule module);
#endif
