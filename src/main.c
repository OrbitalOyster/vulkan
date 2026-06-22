#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

GLFWwindow *GLFWWindow;
VkInstance vulkanInstance;
VkDevice vulkanLogicalDevice;
VkSurfaceKHR vulkanSurface;
VkSwapchainKHR swapChain;

#define VALIDATION_LAYERS_COUNT 1

int main(void) {
  /* Init GLFW */
  if (!glfwInit())
    PANIC(1, "Unable to init GLFW")
  INFO("Initialized GLFW")

  /* Create GLFW window */
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  GLFWWindow = glfwCreateWindow(640, 480, "Vulkan", NULL, NULL);
  if (!GLFWWindow)
    PANIC(1, "Unable to create GLFW window")
  glfwMakeContextCurrent(GLFWWindow);
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

  /* The only device */
  if (deviceCount == 1)
    physicalDevice = physicalDevices[0];
  /* Else - found more suitable */
  else
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
  VkPhysicalDeviceFeatures deviceFeatures = {0};

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
  VkDeviceCreateInfo createLogicalDeviceInfo = {
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

  if (vkCreateDevice(physicalDevice, &createLogicalDeviceInfo, NULL,
                     &vulkanLogicalDevice) == VK_SUCCESS) {
    INFO("Created Vulkan logical device")
  } else
    PANIC(1, "Unable to create Vulkan logical device")

  /* Graphics queue */
  VkQueue graphicsQueue;
  vkGetDeviceQueue(vulkanLogicalDevice, selectedQueueFamilyIndex, 0,
                   &graphicsQueue);

  /* Vulkan surface */
  if (glfwCreateWindowSurface(vulkanInstance, GLFWWindow, NULL,
                              &vulkanSurface) == VK_SUCCESS) {
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
  glfwGetFramebufferSize(GLFWWindow, &windowWidth, &windowHeight);
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
  VkExtent2D swapchainExtent =
      extent_suitable ? surfaceCapabilities.currentExtent : actualExtent;

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
      .imageExtent = swapchainExtent,
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
  vkGetSwapchainImagesKHR(vulkanLogicalDevice, swapChain, &imageCount,
                          swapChainImages);

  /* Swap chain image views */
  VkImageView *swapChainImageViews = calloc(sizeof(VkImageView), imageCount);
  for (size_t i = 0; i < imageCount; i++) {
    VkImageViewCreateInfo createImageViewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = swapChainImages[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = surfaceFormat.format,
        .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
    };
    if (vkCreateImageView(vulkanLogicalDevice, &createImageViewInfo, NULL,
                          &swapChainImageViews[i]) != VK_SUCCESS)
      PANIC(1, "Failed to create image view")
  }

  /* Load shaders */
  FILE *vertexShaderFile = NULL, *fragmentShaderFile = NULL;
  vertexShaderFile = fopen("shaders/vert.spv", "rb+");
  fragmentShaderFile = fopen("shaders/frag.spv", "rb+");
  if (vertexShaderFile == NULL || fragmentShaderFile == NULL) {
    PANIC(1, "Unable to load shaders")
  }
  fseek(vertexShaderFile, 0, SEEK_END);
  fseek(fragmentShaderFile, 0, SEEK_END);
  uint32_t vertexShaderFileSize = ftell(vertexShaderFile);
  uint32_t fragmentShaderFileSize = ftell(fragmentShaderFile);
  char *vertexShaderCode = (char *)calloc(vertexShaderFileSize, sizeof(char));
  char *fragmentShaderCode =
      (char *)calloc(fragmentShaderFileSize, sizeof(char));
  rewind(vertexShaderFile);
  rewind(fragmentShaderFile);
  fread(vertexShaderCode, 1, vertexShaderFileSize, vertexShaderFile);
  INFOF("Loaded vertex shaders, %u bytes", vertexShaderFileSize)
  fread(fragmentShaderCode, 1, fragmentShaderFileSize, fragmentShaderFile);
  INFOF("Loaded fragment shaders, %u bytes", fragmentShaderFileSize)
  fclose(vertexShaderFile);
  fclose(fragmentShaderFile);

  /* Shader modules */
  VkShaderModuleCreateInfo createVertexShaderInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = vertexShaderFileSize,
      .pCode = (const uint32_t *)vertexShaderCode,
  };
  VkShaderModule vertexShaderModule;
  if (vkCreateShaderModule(vulkanLogicalDevice, &createVertexShaderInfo, NULL,
                           &vertexShaderModule) != VK_SUCCESS)
    PANIC(1, "Failed to create vertex shader module")
  VkShaderModuleCreateInfo createFragmentShaderInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = fragmentShaderFileSize,
      .pCode = (const uint32_t *)fragmentShaderCode,
  };
  VkShaderModule fragmentShaderModule;
  if (vkCreateShaderModule(vulkanLogicalDevice, &createFragmentShaderInfo, NULL,
                           &fragmentShaderModule) != VK_SUCCESS)
    PANIC(1, "Failed to create fragment shader module")

  /* Shader staging */
  VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vertexShaderModule,
      .pName = "main",
  };

  VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = fragmentShaderModule,
      .pName = "main",
  };

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo,
                                                    fragmentShaderStageInfo};

  /* Pipeline setup */
  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 0,
      .pVertexBindingDescriptions = NULL,
      .vertexAttributeDescriptionCount = 0,
      .pVertexAttributeDescriptions = NULL,
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };

  VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = (float)swapchainExtent.width,
      .height = (float)swapchainExtent.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };

  VkRect2D scissor = {
      .offset = {0, 0},
      .extent = swapchainExtent,
  };

  /* TODO: No magic numbers */
  VkDynamicState dynamicStates[2] = {VK_DYNAMIC_STATE_VIEWPORT,
                                     VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = 2,
      .pDynamicStates = dynamicStates,
  };

  VkPipelineViewportStateCreateInfo viewportState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = &viewport,
      .scissorCount = 1,
      .pScissors = &scissor,
  };

  /* Rasterizer */
  VkPipelineRasterizationStateCreateInfo rasterizer = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .lineWidth = 1.0f,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .depthBiasConstantFactor = 0.0f,
      .depthBiasClamp = 0.0f,
      .depthBiasSlopeFactor = 0.0f,
  };

  /* Multisampling */
  VkPipelineMultisampleStateCreateInfo multisampling = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .sampleShadingEnable = VK_FALSE,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .minSampleShading = 1.0f,
      .pSampleMask = NULL,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable = VK_FALSE,
  };

  /* Color blending */
  VkPipelineColorBlendAttachmentState colorBlendAttachment = {
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
      .blendEnable = VK_FALSE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp = VK_BLEND_OP_ADD,
  };

  VkPipelineLayout pipelineLayout;
  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pSetLayouts = NULL,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = NULL,
  };

  if (vkCreatePipelineLayout(vulkanLogicalDevice, &pipelineLayoutInfo, NULL,
                             &pipelineLayout) != VK_SUCCESS)
    PANIC(1, "Failed to create pipeline")

  /* Render pass */
  VkAttachmentDescription colorAttachment = {
      .format = surfaceFormat.format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };

  VkPipelineColorBlendStateCreateInfo colorBlending = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment,
      .blendConstants[0] = 0.0f,
      .blendConstants[1] = 0.0f,
      .blendConstants[2] = 0.0f,
      .blendConstants[3] = 0.0f,
  };

  VkAttachmentReference colorAttachmentRef = {
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };

  VkSubpassDescription subpass = {
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentRef,
  };

  /* Create render pass */
  VkRenderPassCreateInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &colorAttachment,
      .subpassCount = 1,
      .pSubpasses = &subpass,
  };

  VkRenderPass renderPass;
  if (vkCreateRenderPass(vulkanLogicalDevice, &renderPassInfo, NULL,
                         &renderPass) != VK_SUCCESS)
    PANIC(1, "Failed to create render pass")

  VkGraphicsPipelineCreateInfo pipelineInfo = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = 2,
      .pStages = shaderStages,
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pDepthStencilState = NULL,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicState,
      .layout = pipelineLayout,
      .renderPass = renderPass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1,
  };

  INFO("Starting cleanup")
  vkDestroyPipelineLayout(vulkanLogicalDevice, pipelineLayout, NULL);
  vkDestroyRenderPass(vulkanLogicalDevice, renderPass, NULL);
  vkDestroyPipelineLayout(vulkanLogicalDevice, pipelineLayout, NULL);
  vkDestroyShaderModule(vulkanLogicalDevice, vertexShaderModule, NULL);
  vkDestroyShaderModule(vulkanLogicalDevice, fragmentShaderModule, NULL);
  for (size_t i = 0; i < imageCount; i++)
    vkDestroyImageView(vulkanLogicalDevice, swapChainImageViews[i], NULL);
  vkDestroySwapchainKHR(vulkanLogicalDevice, swapChain, NULL);
  vkDestroyDevice(vulkanLogicalDevice, NULL);
  vkDestroySurfaceKHR(vulkanInstance, vulkanSurface, NULL);
  vkDestroyInstance(vulkanInstance, NULL);
  glfwDestroyWindow(GLFWWindow);
  glfwTerminate();
  INFO("Cleanup complete")

  return EXIT_SUCCESS;
}
