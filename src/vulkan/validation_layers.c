#include "vulkan/validation_layers.h"
#include "debug.h"
#include "hardcoded.h"
#include <stdbool.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

const char *required_validation_layers[VALIDATION_LAYER_COUNT] = {
    "VK_LAYER_KHRONOS_validation"};

const char **get_required_validation_layers(void) {
  return required_validation_layers;
}

/* Validation layers */
void enable_vulkan_validation_layers(void) {
  /* Get number of layers */
  uint32_t available_layer_count = 0;
  vkEnumerateInstanceLayerProperties(&available_layer_count, VK_NULL_HANDLE);
  // INFOF("Available Vulkan validation layers: %u", available_layer_count)
  VkLayerProperties *layer_properties =
      calloc(sizeof(VkLayerProperties), available_layer_count);
  /* Get layer properties */
  vkEnumerateInstanceLayerProperties(&available_layer_count, layer_properties);
  // for (uint32_t i = 0; i < available_layer_count; i++)
  //  INFOF("\t%s - %s", layer_properties[i].layerName,
  //        layer_properties[i].description)
  /* Check if all requested validation layers are available */
  for (uint32_t i = 0; i < VALIDATION_LAYER_COUNT; i++) {
    const char *layer_name = required_validation_layers[i];
    bool layer_found = false;
    for (uint32_t j = 0; j < available_layer_count; j++) {
      if (strcmp(layer_name, layer_properties[j].layerName) == 0) {
        layer_found = true;
        break;
      }
    }
    if (!layer_found)
      PANICF(1, "Validation layer not found: %s", layer_name)
  }
}
