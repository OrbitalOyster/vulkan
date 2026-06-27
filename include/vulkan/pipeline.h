#ifndef DONGLE_VULKAN_PIPELINE
#define DONGLE_VULKAN_PIPELINE

#include <vulkan/vulkan_core.h>

/* Constructors */

VkPipelineLayout create_pipeline_layout(VkDevice logical_device);
VkRenderPass create_render_pass(VkSurfaceFormatKHR surface_format,
                                VkDevice logical_device);
VkPipeline create_pipeline(VkViewport *viewport, VkRect2D *scissor,
                           VkDevice logical_device,
                           VkPipelineShaderStageCreateInfo *shader_stages,
                           VkPipelineLayout pipeline_layout,
                           VkRenderPass render_pass);
/* Destructors */

void destroy_pipeline_layout(VkDevice logical_device,
                             VkPipelineLayout pipeline_layout);
void destroy_render_pass(VkDevice logical_device, VkRenderPass render_pass);
void destroy_pipeline(VkDevice logical_device, VkPipeline pipeline);
#endif
