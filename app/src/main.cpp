#include <GLFW/glfw3.h>
#include <stdio.h>

#include "window_main.h"

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

namespace otterboard::app {
int main() {

  // Setup error callback
  glfwSetErrorCallback(glfw_error_callback);

  // Initialize GLFW
  if (!glfwInit())
    return 1;

  window_main::setup setup;
  window_main::state state;
  window_main window(setup, state);
  return window.get_exit_code();
}
} // namespace otterboard::app

int main() { return otterboard::app::main(); }
