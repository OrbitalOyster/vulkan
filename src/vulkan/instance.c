#include "vulkan/instance.h"
#include "debug.h"
#include "hardcoded.h"

VkInstance create_vulkan_instance(uint32_t glfw_extension_count,
                                  const char **glfw_extension_names,
                                  const char **required_validation_layers) {
  /* App info */
  VkApplicationInfo app_info = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                .pApplicationName = APP_NAME,
                                .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
                                .pEngineName = "No engine",
                                .engineVersion = VK_MAKE_VERSION(0, 0, 1),
                                .apiVersion = VK_API_VERSION_1_3};
  /* Vulkan info */
  VkInstanceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &app_info,
      .enabledExtensionCount = glfw_extension_count,
      .ppEnabledExtensionNames = glfw_extension_names,
      .enabledLayerCount = VALIDATION_LAYER_COUNT,
      .ppEnabledLayerNames = required_validation_layers,
  };
  /* Get Vulkan extension count */
  uint32_t vulkan_extension_count = 0;
  vkEnumerateInstanceExtensionProperties(
      VK_NULL_HANDLE, &vulkan_extension_count, VK_NULL_HANDLE);
  /* Get extensions */
  VkExtensionProperties *vulkan_extension_properties =
      calloc(sizeof(VkExtensionProperties), vulkan_extension_count);
  vkEnumerateInstanceExtensionProperties(
      VK_NULL_HANDLE, &vulkan_extension_count, vulkan_extension_properties);
  VkInstance instance = VK_NULL_HANDLE;
  if (vkCreateInstance(&create_info, VK_NULL_HANDLE, &instance) != VK_SUCCESS)
    PANIC(1, "Unable to create Vulkan instance")
  return instance;
}

void destroy_vulkan_instance(VkInstance *instance) {
  vkDestroyInstance(*instance, VK_NULL_HANDLE);
}
