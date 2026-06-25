#include "debug.h"
#include "glfw.h"
#include "vulkan/instance.h"
#include "vulkan/physical_device.h"
#include "vulkan/queue.h"
#include "vulkan/validation_layers.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

VkDevice vulkanLogicalDevice;
VkSurfaceKHR vulkanSurface;
VkSwapchainKHR swapChain;

#define MAX_FRAMES_IN_FLIGHT 2

int main(void) {
  init_glfw();
  INFO("Initialized GLFW")
  GLFWwindow *glfw_window = create_window();
  INFO("Created window")
  uint32_t glfw_extension_count = get_glfw_extension_count();
  INFOF("GLFW extensions: %i", glfw_extension_count)
  const char **glfw_extension_names =
      get_glfw_extension_names(&glfw_extension_count);
  INFOF("GLFW extensions: %i", glfw_extension_count)
  for (uint32_t i = 0; i < glfw_extension_count; i++)
    INFOF("\t%s", glfw_extension_names[i])
  enable_vulkan_validation_layers();
  const char **required_validation_layers = get_required_validation_layers();
  INFO("Enabled validation layers")
  VkInstance vulkan_instance = create_vulkan_instance(
      glfw_extension_count, glfw_extension_names, required_validation_layers);
  INFO("Created Vulkan instance")
  VkPhysicalDevice physical_device = select_physical_device(vulkan_instance);
  INFO("Selected physical device");
  uint32_t queue_family_count = get_queue_family_count(physical_device);
  INFOF("Queue families: %u", queue_family_count)
  uint32_t selectedQueueFamilyIndex =
      select_queue_family(physical_device, queue_family_count);

  /* Queue priorities */
  // float *queuePriorities =
  //     calloc(sizeof(float), queueCount[selectedQueueFamilyIndex]);
  // for (uint32_t i = 0; i < queueCount[selectedQueueFamilyIndex]; i++)
  //   queuePriorities[i] = 1.0;

  float queuePriority = 1.0;
  /* Queue create info */
  VkDeviceQueueCreateInfo queueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .queueFamilyIndex = selectedQueueFamilyIndex,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority,
  };

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
  vkEnumerateDeviceExtensionProperties(physical_device, NULL,
                                       &availableExtentionsCount, NULL);
  INFOF("Available extensions: %u", availableExtentionsCount)
  VkExtensionProperties *availableExtensions =
      calloc(sizeof(VkExtensionProperties), availableExtentionsCount);
  vkEnumerateDeviceExtensionProperties(
      physical_device, NULL, &availableExtentionsCount, availableExtensions);

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
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = NULL,
      .pEnabledFeatures = &deviceFeatures,
      .enabledExtensionCount = requiredExtensionCount,
      .ppEnabledExtensionNames = requiredExtensionNames,
  };

  if (vkCreateDevice(physical_device, &createLogicalDeviceInfo, NULL,
                     &vulkanLogicalDevice) == VK_SUCCESS) {
    INFO("Created Vulkan logical device")
  } else
    PANIC(1, "Unable to create Vulkan logical device")

  /* Graphics queue */
  VkQueue graphicsQueue;
  vkGetDeviceQueue(vulkanLogicalDevice, selectedQueueFamilyIndex, 0,
                   &graphicsQueue);

  /* Vulkan surface */
  if (glfwCreateWindowSurface(vulkan_instance, glfw_window, NULL,
                              &vulkanSurface) == VK_SUCCESS) {
    INFO("Created Vulkan surface")
  } else
    PANIC(1, "Unable to create Vulkan surface")

  /* Verify surface support */
  VkBool32 surfaceSupported;
  vkGetPhysicalDeviceSurfaceSupportKHR(physical_device,
                                       selectedQueueFamilyIndex, vulkanSurface,
                                       &surfaceSupported);
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
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, vulkanSurface,
                                            &surfaceCapabilities);

  /* Surface formats */
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vulkanSurface,
                                       &formatCount, NULL);
  INFOF("Surface formats: %u", formatCount)
  if (formatCount == 0)
    PANIC(1, "No surface formats available")
  VkSurfaceFormatKHR *formats = calloc(sizeof(VkSurfaceFormatKHR), formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vulkanSurface,
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
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vulkanSurface,
                                            &presentModeCount, NULL);
  INFOF("Present modes: %u", presentModeCount)
  if (presentModeCount == 0)
    PANIC(1, "No present modes available")
  VkPresentModeKHR *presentModes =
      calloc(sizeof(VkPresentModeKHR), presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vulkanSurface,
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
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, vulkanSurface,
                                            &surfaceCapabilities);
  bool extent_suitable = true;
  int windowWidth, windowHeight;
  glfwGetFramebufferSize(glfw_window, &windowWidth, &windowHeight);
  INFOF("GLFW framebuffer size: %u %u", windowWidth, windowHeight)
  VkExtent2D actualExtent = {
      .width = windowWidth,
      .height = windowHeight,
  };
  actualExtent.width = windowWidth;
  actualExtent.height = windowHeight;
  INFOF("Surface capabilities: %u %u", surfaceCapabilities.currentExtent.width,
        surfaceCapabilities.currentExtent.height)
  /* TODO: Wayland will return 0xFFFFFFFF on both */
  if (surfaceCapabilities.currentExtent.width != (uint32_t)windowWidth ||
      surfaceCapabilities.currentExtent.height != (uint32_t)windowHeight) {
    INFO("Resizing swapchain extent")
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

  INFOF("Swapchain extent: %ux%u", swapchainExtent.width,
        swapchainExtent.height)

  /* Create swapchain */
  uint32_t minSwapchainImages = surfaceCapabilities.minImageCount;
  INFOF("Minimum swapchain images: %u", minSwapchainImages);
  uint32_t maxSwapchainImages = surfaceCapabilities.maxImageCount;
  INFOF("Maximum swapchain images: %u", maxSwapchainImages);
  uint32_t imageCount = minSwapchainImages + 1;
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
                          &swapChainImageViews[i]) == VK_SUCCESS) {
      INFOF("Created image view #%u", i)
    } else
      PANIC(1, "Failed to create image view")
  }

  /* Load shaders */
  FILE *vertexShaderFile = NULL, *fragmentShaderFile = NULL;
  vertexShaderFile = fopen("shaders/vert.spv", "rb+");
  fragmentShaderFile = fopen("shaders/frag.spv", "rb+");
  if (vertexShaderFile != NULL && fragmentShaderFile != NULL) {
    INFO("Loaded shaders code")
  } else
    PANIC(1, "Unable to load shaders")
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
                           &vertexShaderModule) == VK_SUCCESS) {
    INFO("Created vertex shader module")
  } else
    PANIC(1, "Failed to create vertex shader module")
  VkShaderModuleCreateInfo createFragmentShaderInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = fragmentShaderFileSize,
      .pCode = (const uint32_t *)fragmentShaderCode,
  };
  VkShaderModule fragmentShaderModule;
  if (vkCreateShaderModule(vulkanLogicalDevice, &createFragmentShaderInfo, NULL,
                           &fragmentShaderModule) == VK_SUCCESS) {

    INFO("Created fragment shader module")
  } else
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
      .width = swapchainExtent.width,
      .height = swapchainExtent.height,
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
                             &pipelineLayout) == VK_SUCCESS) {
    INFO("Created pipeline layout")
  } else
    PANIC(1, "Failed to create pipeline layout")

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

  /* Render pass dependency */
  VkSubpassDependency dependency = {
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  };

  /* Create render pass */
  VkRenderPassCreateInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &colorAttachment,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &dependency,
  };

  VkRenderPass renderPass;
  if (vkCreateRenderPass(vulkanLogicalDevice, &renderPassInfo, NULL,
                         &renderPass) == VK_SUCCESS) {
    INFO("Created render pass")
  } else
    PANIC(1, "Failed to create render pass")

  VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
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

  /* Actual pipeline */
  VkPipeline graphicsPipeline;
  if (vkCreateGraphicsPipelines(vulkanLogicalDevice, VK_NULL_HANDLE, 1,
                                &pipelineCreateInfo, NULL,
                                &graphicsPipeline) == VK_SUCCESS) {
    INFO("Created pipeline")
  } else
    PANIC(1, "Failed to create graphics pipeline")

  /* Framebuffers */
  VkFramebuffer *swapChainFramebuffers =
      calloc(sizeof(VkFramebuffer), imageCount);
  for (size_t i = 0; i < imageCount; i++) {
    VkImageView attachments[] = {swapChainImageViews[i]};

    VkFramebufferCreateInfo framebufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = renderPass,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .width = swapchainExtent.width,
        .height = swapchainExtent.height,
        .layers = 1,
    };

    if (vkCreateFramebuffer(vulkanLogicalDevice, &framebufferCreateInfo, NULL,
                            &swapChainFramebuffers[i]) != VK_SUCCESS)
      PANIC(1, "Failed to create framebuffer")
  }
  INFOF("Created %u framebuffers", imageCount)

  /* Command pools */
  VkCommandPool commandPool;
  VkCommandPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = selectedQueueFamilyIndex,
  };

  if (vkCreateCommandPool(vulkanLogicalDevice, &poolInfo, NULL, &commandPool) ==
      VK_SUCCESS) {
    INFO("Created command pool")
  } else
    PANIC(1, "Unable to create command pool")

  /* Command buffer */

  /* Allocate */
  VkCommandBuffer commandBuffer;
  VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };
  if (vkAllocateCommandBuffers(vulkanLogicalDevice, &allocInfo,
                               &commandBuffer) == VK_SUCCESS) {
    INFO("Allocated command buffers")
  } else
    PANIC(1, "Unable to allocate command buffers");

  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

  /* Semaphores */
  VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };

  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;
  VkFence inFlightFence;

  if (vkCreateSemaphore(vulkanLogicalDevice, &semaphoreInfo, NULL,
                        &imageAvailableSemaphore) == VK_SUCCESS &&
      vkCreateSemaphore(vulkanLogicalDevice, &semaphoreInfo, NULL,
                        &renderFinishedSemaphore) == VK_SUCCESS &&
      vkCreateFence(vulkanLogicalDevice, &fenceInfo, NULL, &inFlightFence) ==
          VK_SUCCESS) {
    INFO("Created semaphores")
  } else
    PANIC(1, "Failed to create semaphores")

  while (!glfwWindowShouldClose(glfw_window)) {
    glfwPollEvents();

    /* Draw frame */
    vkWaitForFences(vulkanLogicalDevice, 1, &inFlightFence, VK_TRUE,
                    UINT64_MAX);
    vkResetFences(vulkanLogicalDevice, 1, &inFlightFence);
    uint32_t imageIndex;
    vkAcquireNextImageKHR(vulkanLogicalDevice, swapChain, UINT64_MAX,
                          imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(commandBuffer, 0);

    // recordCommandBuffer

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = NULL,
    };
    if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) ==
        VK_SUCCESS) {
      // INFO("Command buffer start")
    } else
      PANIC(1, "Unable to start recording command buffer")

    VkRenderPassBeginInfo renderPassBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass,
        .framebuffer = swapChainFramebuffers[imageIndex],
        .renderArea.offset = {0, 0},
        .renderArea.extent = swapchainExtent,
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphicsPipeline);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) == VK_SUCCESS) {
      // INFO("Command buffer end")
    } else {
      PANIC(1, "Failed to record command buffer")
    }
    // ----------------------------------------------

    /* Submit command buffer */
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) !=
        VK_SUCCESS)
      PANIC(1, "Failed to submit draw command buffer")

    /* Present */
    VkSwapchainKHR swapChains[] = {swapChain};
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,
        .pResults = NULL,
    };
    /* Ta-da */
    vkQueuePresentKHR(presentQueue, &presentInfo);
  }

  vkDeviceWaitIdle(vulkanLogicalDevice);

  INFO("Starting cleanup")
  vkDestroySemaphore(vulkanLogicalDevice, imageAvailableSemaphore, NULL);
  vkDestroySemaphore(vulkanLogicalDevice, renderFinishedSemaphore, NULL);
  vkDestroyCommandPool(vulkanLogicalDevice, commandPool, NULL);
  vkDestroyFence(vulkanLogicalDevice, inFlightFence, NULL);
  for (uint32_t i = 0; i < imageCount; i++)
    vkDestroyFramebuffer(vulkanLogicalDevice, swapChainFramebuffers[i], NULL);
  vkDestroyPipeline(vulkanLogicalDevice, graphicsPipeline, NULL);
  vkDestroyPipelineLayout(vulkanLogicalDevice, pipelineLayout, NULL);
  vkDestroyRenderPass(vulkanLogicalDevice, renderPass, NULL);
  vkDestroyShaderModule(vulkanLogicalDevice, vertexShaderModule, NULL);
  vkDestroyShaderModule(vulkanLogicalDevice, fragmentShaderModule, NULL);
  for (size_t i = 0; i < imageCount; i++)
    vkDestroyImageView(vulkanLogicalDevice, swapChainImageViews[i], NULL);
  vkDestroySwapchainKHR(vulkanLogicalDevice, swapChain, NULL);
  vkDestroyDevice(vulkanLogicalDevice, NULL);
  // vkDestroySurfaceKHR(vulkanInstance, vulkanSurface, NULL);
  INFO("Cleanup complete")

  return EXIT_SUCCESS;
}
