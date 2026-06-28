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

void begin_render_pass(VkRenderPass render_pass, VkFramebuffer framebuffer,
                       VkExtent2D extent, VkCommandBuffer command_buffer,
                       VkPipeline pipeline, VkViewport *viewport,
                       VkRect2D *scissor);

void end_render_pass(VkCommandBuffer command_buffer);

/* Destructors */

void destroy_pipeline_layout(VkDevice logical_device,
                             VkPipelineLayout pipeline_layout);
void destroy_render_pass(VkDevice logical_device, VkRenderPass render_pass);
void destroy_pipeline(VkDevice logical_device, VkPipeline pipeline);
#endif
