#include "vulkan/physical_device.h"
#include "debug.h"

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
