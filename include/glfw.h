#ifndef DONGLE_GLFW
#define DONGLE_GLFW

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

void init_glfw(void);
GLFWwindow *create_window(void);
uint32_t get_glfw_extension_count(void);
const char **get_glfw_extension_names(uint32_t *glfw_extension_count);
void destroy_window(GLFWwindow *window);

#endif
