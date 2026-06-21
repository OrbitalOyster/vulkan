#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

GLFWwindow *window;
VkInstance vulkanInstance;
VkDevice vulkanLogicalDevice;
VkSurfaceKHR vulkanSurface;

#define VALIDATION_LAYERS_COUNT 1

void cleanup(void) {
  INFO("Starting cleanup")
  vkDestroyDevice(vulkanLogicalDevice, NULL);
  vkDestroySurfaceKHR(vulkanInstance, vulkanSurface, NULL);
  vkDestroyInstance(vulkanInstance, NULL);
  glfwDestroyWindow(window);
  glfwTerminate();
  INFO("Cleanup complete")
}

int main(void) {
  /* Init GLFW */
  if (!glfwInit())
    PANIC(1, "Unable to init GLFW")
  INFO("Initialized GLFW")

  /* Create GLFW window */
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  window = glfwCreateWindow(640, 480, "Vulkan", NULL, NULL);
  if (!window)
    PANIC(1, "Unable to create GLFW window")
  glfwMakeContextCurrent(window);
  INFO("Created window")

  /* Check if vulkan is supported */
  if (glfwVulkanSupported())
    INFO("Vulkan supported")

  /* GLFW extensions */
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions =
      glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  INFOF("GLFW extensions: %i", glfwExtensionCount)
  for (uint32_t i = 0; i < glfwExtensionCount; i++)
    INFOF("\t%s", glfwExtensions[i])

  /* Validation layers */
  uint32_t availableValidationLayerCount;
  vkEnumerateInstanceLayerProperties(&availableValidationLayerCount, NULL);
  INFOF("Vulkan validation layers: %u", availableValidationLayerCount)
  VkLayerProperties *availableValidationLayers =
      calloc(sizeof(VkLayerProperties), availableValidationLayerCount);
  vkEnumerateInstanceLayerProperties(&availableValidationLayerCount,
                                     availableValidationLayers);
  for (uint32_t i = 0; i < availableValidationLayerCount; i++)
    INFOF("\t%s - %s", availableValidationLayers[i].layerName,
          availableValidationLayers[i].description)

  const char *validationLayers[VALIDATION_LAYERS_COUNT] = {
      "VK_LAYER_KHRONOS_validation"};

  /* Check if all requested validation layers are available */
  for (uint32_t i = 0; i < VALIDATION_LAYERS_COUNT; i++) {
    const char *layerName = validationLayers[i];
    bool layerFound = false;
    for (uint32_t j = 0; j < availableValidationLayerCount; j++) {
      if (strcmp(layerName, availableValidationLayers[j].layerName) == 0) {
        layerFound = true;
        break;
      }
    }
    if (!layerFound) {
      PANICF(1, "Validation layer not found: %s", layerName)
    }
  }

  /* Gather vulkan info */
  VkApplicationInfo appInfo = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                               .pApplicationName = "Vulkan",
                               .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                               .pEngineName = "No Engine",
                               .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                               .apiVersion = VK_API_VERSION_1_0};
  VkInstanceCreateInfo createVulkanInstanceInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
      .enabledExtensionCount = glfwExtensionCount,
      .ppEnabledExtensionNames = glfwExtensions,
      .enabledLayerCount = VALIDATION_LAYERS_COUNT,
      .ppEnabledLayerNames = validationLayers,
  };

  /* Vulkan extensions */
  uint32_t vulkanExtensionCount = 0;
  vkEnumerateInstanceExtensionProperties(NULL, &vulkanExtensionCount, NULL);
  INFOF("Vulkan extensions: %u", vulkanExtensionCount)
  VkExtensionProperties *vulkanExtensionProperties =
      calloc(sizeof(VkExtensionProperties), vulkanExtensionCount);
  vkEnumerateInstanceExtensionProperties(NULL, &vulkanExtensionCount,
                                         vulkanExtensionProperties);
  for (uint32_t i = 0; i < vulkanExtensionCount; i++)
    INFOF("\t%s", vulkanExtensionProperties[i].extensionName)

  /* Create vulkan instance */
  if (vkCreateInstance(&createVulkanInstanceInfo, NULL, &vulkanInstance) ==
      VK_SUCCESS) {
    INFO("Created Vulkan instance")
  } else
    PANIC(1, "Unable to create Vulkan instance")

  /* Physical device */
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, NULL);
  INFOF("Physical devices: %u", deviceCount)
  if (deviceCount == 0)
    PANIC(1, "No physical device found")
  VkPhysicalDevice *physicalDevices =
      calloc(sizeof(VkPhysicalDevice), deviceCount);
  vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, physicalDevices);
  for (uint32_t i = 0; i < deviceCount; i++) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevices[i], &deviceFeatures);
    INFOF("\t%s", deviceProperties.deviceName)
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      INFO("\t\tLooks good")
      physicalDevice = physicalDevices[i];
    } else
      INFO("\t\tIgnored")
  }
  /* Nothing good found */
  if (physicalDevice == VK_NULL_HANDLE)
    PANIC(1, "No suitable device found")

  /* Queue families */
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           NULL);
  INFOF("Queue families: %u", queueFamilyCount)
  VkQueueFamilyProperties *queueFamilies =
      calloc(sizeof(VkQueueFamilyProperties), queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           queueFamilies);

  /* Queue count */
  uint32_t queueCount[queueFamilyCount];
  uint32_t selectedQueueIndex = 0;
  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    queueCount[i] = queueFamilies[i].queueCount;
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      selectedQueueIndex = i;
      INFOF("\tSelected queue family index: %u, queue count: %u",
            selectedQueueIndex, queueCount[i])
    }
  }

  /* Queue priorities */
  float *queuePriorities =
      calloc(sizeof(float), queueCount[selectedQueueIndex]);
  for (uint32_t i = 0; i < queueCount[selectedQueueIndex]; i++)
    queuePriorities[i] = 1.0;

  /* Queue create info */
  VkDeviceQueueCreateInfo *queueCreateInfo =
      calloc(sizeof(VkDeviceQueueCreateInfo), queueFamilyCount);
  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    VkDeviceQueueCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueFamilyIndex = i,
        .queueCount = queueCount[selectedQueueIndex],
        .pQueuePriorities = queuePriorities,
    };
    queueCreateInfo[i] = info;
  }

  /* Physical device features */
  VkPhysicalDeviceFeatures deviceFeatures = {
      .geometryShader = VK_TRUE,
  };

  /* Logical device */
  VkDeviceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .queueCreateInfoCount = queueFamilyCount,
      .pQueueCreateInfos = queueCreateInfo,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = NULL,
      .pEnabledFeatures = &deviceFeatures,
      .enabledExtensionCount = 0,
  };

  if (vkCreateDevice(physicalDevice, &createInfo, NULL, &vulkanLogicalDevice) ==
      VK_SUCCESS) {
    INFO("Created Vulkan logical device")
  } else
    PANIC(1, "Unable to create Vulkan logical device")

  /* Graphics queue */
  VkQueue graphicsQueue;
  vkGetDeviceQueue(vulkanLogicalDevice, selectedQueueIndex, 0, &graphicsQueue);

  /* Vulkan surface */
  if (glfwCreateWindowSurface(vulkanInstance, window, NULL, &vulkanSurface) ==
      VK_SUCCESS) {
    INFO("Created Vulkan surface")
  } else
    PANIC(1, "Unable to create Vulkan surface")

  // while (!glfwWindowShouldClose(window)) {
  // glClear(GL_COLOR_BUFFER_BIT);
  // glfwSwapBuffers(window);
  // glfwPollEvents();
  // }

  free(availableValidationLayers);
  free(vulkanExtensionProperties);

  cleanup();

  return EXIT_SUCCESS;
}
