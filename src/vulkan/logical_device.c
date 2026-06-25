#include "vulkan/logical_device.h"
#include "debug.h"
#include "hardcoded.h"
#include <stdbool.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

/* Creates logical device */
VkDevice create_logical_device(VkPhysicalDevice physical_device,
                               uint32_t selected_queue_family_index) {

  /* Get avaiable extensions */
  uint32_t available_extension_count = 0;
  vkEnumerateDeviceExtensionProperties(physical_device, VK_NULL_HANDLE,
                                       &available_extension_count,
                                       VK_NULL_HANDLE);
  INFOF("Available extensions: %u", available_extension_count)
  VkExtensionProperties *available_extensions =
      calloc(sizeof(VkExtensionProperties), available_extension_count);
  vkEnumerateDeviceExtensionProperties(physical_device, VK_NULL_HANDLE,
                                       &available_extension_count,
                                       available_extensions);

  /* Required extensions - only swapchain needed */
  const char *required_extension_names[REQUIRED_DEVICE_EXTENSION_COUNT] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  /* Check device extension support */
  for (uint32_t j = 0; j < REQUIRED_DEVICE_EXTENSION_COUNT; j++) {
    bool supported = false;
    for (uint32_t i = 0; i < available_extension_count; i++) {
      if (strcmp(required_extension_names[j],
                 available_extensions[i].extensionName) == 0) {
        supported = true;
        break;
      }
    }
    if (!supported)
      PANICF(1, "Extension %s not supported", required_extension_names[j])
  }

  float queue_priority = 1.0;
  VkDeviceQueueCreateInfo device_queue_create_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = selected_queue_family_index,
      .queueCount = 1,
      .pQueuePriorities = &queue_priority,
  };

  VkDeviceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &device_queue_create_info,
      .ppEnabledLayerNames = VK_NULL_HANDLE,
      .enabledExtensionCount = REQUIRED_DEVICE_EXTENSION_COUNT,
      .ppEnabledExtensionNames = required_extension_names,
  };

  VkDevice logical_device = VK_NULL_HANDLE;
  if (vkCreateDevice(physical_device, &create_info, VK_NULL_HANDLE,
                     &logical_device) != VK_SUCCESS)
    PANIC(1, "Unable to create Vulkan logical device")

  return logical_device;
}
