#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <cmath>

// ============================================================================
// Callbacks
// ============================================================================

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// ============================================================================
// Main
// ============================================================================

int main() {
  // Setup error callback
  glfwSetErrorCallback(glfw_error_callback);

  // Initialize GLFW
  if (!glfwInit()) return 1;

  // GL 3.3 + GLSL 330
  const char *glsl_version = "#version 330";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create window
  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "OtterBoard - ImGui Demo", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // -------------------------------------------------------------------------
  // ImGui initialization
  // -------------------------------------------------------------------------
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // -------------------------------------------------------------------------
  // Application state
  // -------------------------------------------------------------------------
  float my_color[4] = {0.45f, 0.55f, 0.60f, 1.0f};
  bool my_tool_active = true;
  int selected_item = 0;
  float plot_offset = 0.0f;
  char input_buffer[256] = "Type something...";
  bool checked[3] = {true, false, true};

  // -------------------------------------------------------------------------
  // Main loop
  // -------------------------------------------------------------------------
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ---- Window 1: Main Tool ----
    ImGui::Begin("My First Tool", &my_tool_active, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open...", "Ctrl+O")) {
          fprintf(stdout, "Open clicked\n");
        }
        if (ImGui::MenuItem("Save", "Ctrl+S")) {
          fprintf(stdout, "Save clicked\n");
        }
        if (ImGui::MenuItem("Close", "Ctrl+W", nullptr, !my_tool_active)) {
          my_tool_active = false;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit", "Alt+F4")) {
          glfwSetWindowShouldClose(window, true);
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("Dashboard", nullptr, false);
        ImGui::MenuItem("Settings", nullptr, false);
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    // Color editor
    ImGui::Text("Color Picker");
    ImGui::Separator();
    ImGui::ColorEdit4("Background Color", my_color);
    ImGui::SameLine();
    if (ImGui::SmallButton("Reset")) {
      my_color[0] = my_color[1] = my_color[2] = 0.5f;
      my_color[3] = 1.0f;
    }

    // Sliders
    ImGui::Spacing();
    ImGui::Text("Controls");
    ImGui::Separator();
    ImGui::SliderFloat("Plot Speed", &plot_offset, -5.0f, 5.0f, "%.2f");
    ImGui::SliderFloat("Alpha", &my_color[3], 0.0f, 1.0f, "%.2f");

    // Input
    ImGui::Spacing();
    ImGui::Text("Input");
    ImGui::Separator();
    ImGui::InputText("Command", input_buffer, IM_ARRAYSIZE(input_buffer));

    // Checkbox list
    ImGui::Spacing();
    ImGui::Text("Options");
    ImGui::Separator();
    ImGui::Checkbox("Option A", &checked[0]);
    ImGui::Checkbox("Option B", &checked[1]);
    ImGui::Checkbox("Option C", &checked[2]);

    // Combo
    const char *items[] = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon"};
    ImGui::Combo("Item Selector", &selected_item, items,
                 IM_ARRAYSIZE(items));

    // Plot
    ImGui::Spacing();
    ImGui::Text("Live Signal");
    ImGui::Separator();
    static float samples[100];
    for (int n = 0; n < 100; n++)
      samples[n] = sinf(n * 0.2f + ImGui::GetTime() * plot_offset);
    ImGui::PlotLines("Samples", samples, 100, 0,
                     "Sin Wave (realtime)", -1.0f, 1.0f, ImVec2(0, 80));

    // Scrolling text area
    ImGui::Spacing();
    ImGui::Text("Log");
    ImGui::Separator();
    ImGui::BeginChild("Scrolling", ImVec2(0, 120), true);
    for (int n = 0; n < 50; n++) {
      ImGui::TextColored(
          ImVec4(0.6f, 0.8f, 1.0f, 1.0f),
          "%04d: %s", n,
          n % 5 == 0 ? "Info message"
          : n % 5 == 1 ? "Warning message"
          : n % 5 == 2 ? "Debug message"
          : n % 5 == 3 ? "Error message"
          : "Trace message");
    }
    ImGui::EndChild();

    ImGui::End();

    // ---- Window 2: Dashboard ----
    ImGui::Begin("Dashboard", nullptr, ImGuiWindowFlags_None);
    ImGui::Text("OtterBoard v0.0.1");
    ImGui::Separator();
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Window size: %.0fx%.0f", ImGui::GetWindowSize().x,
                ImGui::GetWindowSize().y);

    // Simple button row
    ImGui::Separator();
    ImGui::Text("Quick Actions:");
    if (ImGui::Button("Button 1")) fprintf(stdout, "Button 1 clicked\n");
    ImGui::SameLine();
    if (ImGui::Button("Button 2")) fprintf(stdout, "Button 2 clicked\n");
    ImGui::SameLine();
    if (ImGui::Button("Button 3")) fprintf(stdout, "Button 3 clicked\n");

    // Table
    ImGui::Separator();
    ImGui::Text("Data Table:");
    if (ImGui::BeginTable("SimpleTable", 3, ImGuiTableFlags_Borders)) {
      ImGui::TableSetupColumn("Name");
      ImGui::TableSetupColumn("Value");
      ImGui::TableSetupColumn("Status");
      ImGui::TableHeadersRow();

      const char *names[] = {"Engine", "Renderer", "Audio", "Physics", "Network"};
      for (int i = 0; i < 5; i++) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", names[i]);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", (i + 1) * 100);
        ImGui::TableSetColumnIndex(2);
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "OK");
      }
      ImGui::EndTable();
    }

    ImGui::End();

    // Render
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(my_color[0], my_color[1], my_color[2], my_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // -------------------------------------------------------------------------
  // Cleanup
  // -------------------------------------------------------------------------
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
