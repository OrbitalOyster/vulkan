#include <stdint.h>
#include <vulkan/vulkan_core.h>
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
VkSwapchainKHR swapChain;

#define VALIDATION_LAYERS_COUNT 1

void cleanup(void) {
  INFO("Starting cleanup")
  vkDestroySwapchainKHR(vulkanLogicalDevice, swapChain, NULL);
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

  uint32_t selectedQueueFamilyIndex = 0;
  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    queueCount[i] = queueFamilies[i].queueCount;
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      selectedQueueFamilyIndex = i;
      INFOF("\tSelected queue family index: %u, queue count: %u",
            selectedQueueFamilyIndex, queueCount[i])
    }
  }

  /* Queue priorities */
  float *queuePriorities =
      calloc(sizeof(float), queueCount[selectedQueueFamilyIndex]);
  for (uint32_t i = 0; i < queueCount[selectedQueueFamilyIndex]; i++)
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
        .queueCount = queueCount[selectedQueueFamilyIndex],
        .pQueuePriorities = queuePriorities,
    };
    queueCreateInfo[i] = info;
  }

  /* Physical device features */
  VkPhysicalDeviceFeatures deviceFeatures = {
      .geometryShader = VK_TRUE,
  };

  /* Device extensions */
  uint32_t requiredExtensionCount = 1;
  char tmp[requiredExtensionCount][VK_MAX_EXTENSION_NAME_SIZE];
  strcpy(tmp[0], VK_KHR_SWAPCHAIN_EXTENSION_NAME); // Add extension names here
  // strcpy(tmp[1], "bogus_extension");
  const char *requiredExtensionNames[requiredExtensionCount];
  for (uint32_t i = 0; i < requiredExtensionCount; i++) {
    requiredExtensionNames[i] = tmp[i];
    INFOF("Required extension: %s", requiredExtensionNames[i]);
  }

  /* Check device extension support */
  uint32_t availableExtentionsCount;
  vkEnumerateDeviceExtensionProperties(physicalDevice, NULL,
                                       &availableExtentionsCount, NULL);
  INFOF("Available extensions: %u", availableExtentionsCount)
  VkExtensionProperties *availableExtensions =
      calloc(sizeof(VkExtensionProperties), availableExtentionsCount);
  vkEnumerateDeviceExtensionProperties(
      physicalDevice, NULL, &availableExtentionsCount, availableExtensions);

  for (uint32_t j = 0; j < requiredExtensionCount; j++) {
    bool extensionSupported = false;
    for (uint32_t i = 0; i < availableExtentionsCount; i++) {
      if (strcmp(requiredExtensionNames[j],
                 availableExtensions[i].extensionName) == 0) {
        extensionSupported = true;
        break;
      }
    }
    if (!extensionSupported)
      PANICF(1, "Extension %s not supported", requiredExtensionNames[j])
  }

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
      .enabledExtensionCount = requiredExtensionCount,
      .ppEnabledExtensionNames = requiredExtensionNames,
  };

  if (vkCreateDevice(physicalDevice, &createInfo, NULL, &vulkanLogicalDevice) ==
      VK_SUCCESS) {
    INFO("Created Vulkan logical device")
  } else
    PANIC(1, "Unable to create Vulkan logical device")

  /* Graphics queue */
  VkQueue graphicsQueue;
  vkGetDeviceQueue(vulkanLogicalDevice, selectedQueueFamilyIndex, 0,
                   &graphicsQueue);

  /* Vulkan surface */
  if (glfwCreateWindowSurface(vulkanInstance, window, NULL, &vulkanSurface) ==
      VK_SUCCESS) {
    INFO("Created Vulkan surface")
  } else
    PANIC(1, "Unable to create Vulkan surface")

  /* Verify surface support */
  VkBool32 surfaceSupported;
  vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, selectedQueueFamilyIndex,
                                       vulkanSurface, &surfaceSupported);
  /*
   * TODO: Edge case
   * "It's actually possible that the queue families supporting drawing commands
   * and the ones supporting presentation do not overlap"
   */
  if (surfaceSupported != VK_TRUE)
    PANIC(1, "No surface support")

  /* Present queue */
  VkQueue presentQueue;
  vkGetDeviceQueue(vulkanLogicalDevice, selectedQueueFamilyIndex, 0,
                   &presentQueue);

  /* Surfce capabilities */
  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vulkanSurface,
                                            &surfaceCapabilities);

  /* Surface formats */
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vulkanSurface,
                                       &formatCount, NULL);
  INFOF("Surface formats: %u", formatCount)
  if (formatCount == 0)
    PANIC(1, "No surface formats available")
  VkSurfaceFormatKHR *formats = calloc(sizeof(VkSurfaceFormatKHR), formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vulkanSurface,
                                       &formatCount, formats);

  /* Look for VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR */
  VkSurfaceFormatKHR surfaceFormat;
  for (uint32_t i = 0; i < formatCount; i++) {
    if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
        formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      surfaceFormat = formats[i];
      INFO("Found sRGB surface format")
      break;
    }
  }

  /* Present modes */
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vulkanSurface,
                                            &presentModeCount, NULL);
  INFOF("Present modes: %u", presentModeCount)
  if (presentModeCount == 0)
    PANIC(1, "No present modes available")
  VkPresentModeKHR *presentModes =
      calloc(sizeof(VkPresentModeKHR), presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vulkanSurface,
                                            &presentModeCount, presentModes);
  /* Check present mode */
  VkPresentModeKHR presentMode;
  for (uint32_t i = 0; i < presentModeCount; i++) {
    if (presentModes[i] == VK_PRESENT_MODE_FIFO_KHR) {
      presentMode = presentModes[i];
      INFO("Found present mode")
    }
  }

  /* Swap extent */
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vulkanSurface,
                                            &surfaceCapabilities);
  bool extent_suitable = true;
  int windowWidth, windowHeight;
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
  VkExtent2D actualExtent = {
      .width = windowWidth,
      .height = windowHeight,
  };
  actualExtent.width = windowWidth;
  actualExtent.height = windowHeight;
  if (surfaceCapabilities.currentExtent.width != (uint32_t)windowWidth ||
      surfaceCapabilities.currentExtent.height != (uint32_t)windowHeight) {
    extent_suitable = false;
    actualExtent.width =
        (uint32_t)windowWidth > surfaceCapabilities.maxImageExtent.width
            ? surfaceCapabilities.maxImageExtent.width
            : windowWidth;
    actualExtent.width =
        (uint32_t)windowWidth < surfaceCapabilities.minImageExtent.width
            ? surfaceCapabilities.minImageExtent.width
            : windowWidth;
    actualExtent.height =
        (uint32_t)windowHeight > surfaceCapabilities.maxImageExtent.height
            ? surfaceCapabilities.maxImageExtent.height
            : windowHeight;
    actualExtent.height =
        (uint32_t)windowHeight < surfaceCapabilities.minImageExtent.height
            ? surfaceCapabilities.minImageExtent.height
            : windowHeight;
  }
  VkExtent2D extent = extent_suitable ? surfaceCapabilities.currentExtent : actualExtent;

  /* Create swap chain */
  uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
  if (surfaceCapabilities.maxImageCount > 0 &&
      imageCount > surfaceCapabilities.maxImageCount)
    imageCount = surfaceCapabilities.maxImageCount;

  VkSwapchainCreateInfoKHR createSwapchainInfo = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = vulkanSurface,
      .minImageCount = imageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = surfaceFormat.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .preTransform = surfaceCapabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = presentMode,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE,
  };

  if (vkCreateSwapchainKHR(vulkanLogicalDevice, &createSwapchainInfo, NULL,
                           &swapChain) == VK_SUCCESS) {
    INFO("Created swap chain")
  } else
    PANIC(1, "Unable to create swap chain")

  /* TODO: Manage separate queues edge case */

  /* Swap chain images */
  VkImage *swapChainImages;
  vkGetSwapchainImagesKHR(vulkanLogicalDevice, swapChain, &imageCount, NULL);
  swapChainImages = calloc(sizeof(VkImage), imageCount);
  vkGetSwapchainImagesKHR(vulkanLogicalDevice, swapChain, &imageCount, swapChainImages);

  // while (!glfwWindowShouldClose(window)) {
  // glClear(GL_COLOR_BUFFER_BIT);
  // glfwSwapBuffers(window);
  // glfwPollEvents();
  // }

  // free(availableValidationLayers);
  // free(vulkanExtensionProperties);

  cleanup();

  return EXIT_SUCCESS;
}
