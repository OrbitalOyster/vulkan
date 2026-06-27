#include "vulkan/shaders.h"
#include "debug.h"

VkShaderModule create_shader_module(char *filename, VkDevice logical_device) {
  /* Open and read files */
  FILE *shader_file = NULL;
  shader_file = fopen(filename, "rb+");
  if (shader_file == NULL)
    PANIC(1, "Unable to load shaders")
  fseek(shader_file, 0, SEEK_END);
  uint32_t file_size = ftell(shader_file);
  char *shader_code = (char *)calloc(file_size, sizeof(char));
  rewind(shader_file);
  fread(shader_code, 1, file_size, shader_file);
  fclose(shader_file);
  /* Shader module */
  VkShaderModuleCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = file_size,
      .pCode = (const uint32_t *)shader_code,
  };
  VkShaderModule shader_module = VK_NULL_HANDLE;
  if (vkCreateShaderModule(logical_device, &create_info, VK_NULL_HANDLE,
                           &shader_module) != VK_SUCCESS)
    PANICF(1, "Failed to create shader module %s", filename)
  return shader_module;
}

VkPipelineShaderStageCreateInfo *
create_shader_stages(VkShaderModule vertex_shader_module,
                     VkShaderModule fragment_shader_module) {

  /* Shader staging */
  VkPipelineShaderStageCreateInfo vertex_shader_stage_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vertex_shader_module,
      .pName = "main",
  };

  VkPipelineShaderStageCreateInfo fragment_shader_stage_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = fragment_shader_module,
      .pName = "main",
  };

  VkPipelineShaderStageCreateInfo *shader_stages =
      calloc(sizeof(VkPipelineShaderStageCreateInfo), 2);
  shader_stages[0] = vertex_shader_stage_info;
  shader_stages[1] = fragment_shader_stage_info;

  return shader_stages;
}

void destroy_shader_module(VkDevice logical_device, VkShaderModule module) {
  vkDestroyShaderModule(logical_device, module, VK_NULL_HANDLE);
}
