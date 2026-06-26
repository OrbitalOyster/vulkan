#include "vulkan/physical_device.h"
#include "debug.h"
#include <vulkan/vulkan_core.h>

/* Pick suitable physical device */
VkPhysicalDevice select_physical_device(VkInstance instance) {
  VkPhysicalDevice physical_device = VK_NULL_HANDLE;
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance, &device_count, VK_NULL_HANDLE);
  // INFOF("Physical devices: %u", deviceCount)
  if (device_count == 0)
    PANIC(1, "No physical device found")
  VkPhysicalDevice *all_physical_devices =
      calloc(sizeof(VkPhysicalDevice), device_count);
  vkEnumeratePhysicalDevices(instance, &device_count, all_physical_devices);
  /* The only device */
  if (device_count == 1)
    physical_device = all_physical_devices[0];
  /* Else - found more suitable */
  else
    for (uint32_t i = 0; i < device_count; i++) {
      VkPhysicalDeviceProperties device_properties;
      vkGetPhysicalDeviceProperties(all_physical_devices[i],
                                    &device_properties);
      VkPhysicalDeviceFeatures deviceFeatures;
      vkGetPhysicalDeviceFeatures(all_physical_devices[i], &deviceFeatures);
      // INFOF("\t%s", deviceProperties.deviceName)
      if (device_properties.deviceType ==
          VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        // INFO("\t\tLooks good")
        physical_device = all_physical_devices[i];
      } // else
        // INFO("\t\tIgnored")
    }
  /* Nothing good found */
  if (physical_device == VK_NULL_HANDLE)
    PANIC(1, "No suitable device found");
  return physical_device;
}

VkPresentModeKHR get_present_mode(VkPhysicalDevice physical_device,
                                  VkSurfaceKHR surface) {
  /* Present modes */
  uint32_t present_mode_count = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physical_device, surface, &present_mode_count, VK_NULL_HANDLE);
  INFOF("Present modes: %u", present_mode_count)
  if (present_mode_count == 0)
    PANIC(1, "No present modes available")
  VkPresentModeKHR *all_present_modes =
      calloc(sizeof(VkPresentModeKHR), present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physical_device, surface, &present_mode_count, all_present_modes);
  /* Check present mode */
  VkPresentModeKHR presentMode = {0};
  for (uint32_t i = 0; i < present_mode_count; i++)
    if (all_present_modes[i] == VK_PRESENT_MODE_FIFO_KHR)
      presentMode = all_present_modes[i];
  /* TODO Didn't find present mode */
  return presentMode;
}
