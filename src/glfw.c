#include "glfw.h"
#include "debug.h"
#include "hardcoded.h"
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdlib.h>

/* Inits GLFW */
void init_glfw(void) {
  if (!glfwInit())
    PANIC(1, "Unable to init GLFW")
  /* Check if vulkan is supported */
  if (glfwVulkanSupported() == GLFW_FALSE)
    PANIC(1, "No Vulkan support")
}

void on_window_resize(struct GLFWwindow *window, int width, int height) {
  if (window)
    INFOF("Resized %ix%i", width, height)
}

void on_key(struct GLFWwindow *window, int key, int scancode, int action,
            int mods) {
  INFOF("Key press %i %i %i %i", key, scancode, action, mods)
  if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
    glfwSetWindowShouldClose(window, 1);
  }
}

/* Creates GLFW window */
GLFWwindow *create_window(void) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  GLFWwindow *window =
      glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
  if (!window)
    PANIC(1, "Unable to create GLFW window")
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, on_window_resize);
  glfwSetKeyCallback(window, on_key);
  return window;
}

/* Returns GLFW extension count */
uint32_t get_glfw_extension_count(void) {
  uint32_t glfw_extension_count = 0;
  glfwGetRequiredInstanceExtensions(&glfw_extension_count);
  return glfw_extension_count;
}

/* Loads GLFW extensions */
const char **get_glfw_extension_names(uint32_t *glfw_extension_count) {
  const char **glfw_extensions =
      glfwGetRequiredInstanceExtensions(glfw_extension_count);
  return glfw_extensions;
}

void get_framebuffer_size(GLFWwindow *window, uint32_t *framebuffer_width,
                          uint32_t *framebuffer_height) {
  int w, h;
  glfwGetFramebufferSize(window, &w, &h);
  *framebuffer_width = w;
  *framebuffer_height = h;
}

void destroy_window(GLFWwindow *window) {
  glfwDestroyWindow(window);
  glfwTerminate();
  INFO("Destroyed GLFW window")
}
