#ifndef DONGLE_VULKAN_PIPELINE
#define DONGLE_VULKAN_PIPELINE

#include <vulkan/vulkan_core.h>

VkRenderPass create_render_pass(VkSurfaceFormatKHR surface_format,
                                VkDevice logical_device);
VkPipeline create_graphics_pipeline(
    VkViewport *viewport, VkRect2D *scissor, VkDevice logical_device,
    VkPipelineShaderStageCreateInfo *shader_stages, VkRenderPass render_pass);

#endif
