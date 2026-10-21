
#if 0
int main() {
  glfwPollEvents();

  // Start the ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // ---- Window 1: Main Tool ----
  ImGui::Begin("My First Tool", &state_.my_tool_active,
               ImGuiWindowFlags_MenuBar);
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open...", "Ctrl+O")) {
        fprintf(stdout, "Open clicked\n");
      }
      if (ImGui::MenuItem("Save", "Ctrl+S")) {
        fprintf(stdout, "Save clicked\n");
      }
      if (ImGui::MenuItem("Close", "Ctrl+W", nullptr, !state_.my_tool_active)) {
        state_.my_tool_active = false;
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Exit", "Alt+F4")) {
        glfwSetWindowShouldClose(window_, true);
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
  ImGui::ColorEdit4("Background Color", state_.my_color);
  ImGui::SameLine();
  if (ImGui::SmallButton("Reset")) {
    state_.my_color[0] = state_.my_color[1] = state_.my_color[2] = 0.5f;
    state_.my_color[3] = 1.0f;
  }

  // Sliders
  ImGui::Spacing();
  ImGui::Text("Controls");
  ImGui::Separator();
  ImGui::SliderFloat("Plot Speed", &state_.plot_offset, -5.0f, 5.0f, "%.2f");
  ImGui::SliderFloat("Alpha", &state_.my_color[3], 0.0f, 1.0f, "%.2f");

  // Input
  ImGui::Spacing();
  ImGui::Text("Input");
  ImGui::Separator();
  ImGui::InputText("Command", state_.input_buffer,
                   IM_ARRAYSIZE(state_.input_buffer));

  // Checkbox list
  ImGui::Spacing();
  ImGui::Text("Options");
  ImGui::Separator();
  ImGui::Checkbox("Option A", &state_.checked[0]);
  ImGui::Checkbox("Option B", &state_.checked[1]);
  ImGui::Checkbox("Option C", &state_.checked[2]);

  // Combo
  const char *items[] = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon"};
  ImGui::Combo("Item Selector", &state_.selected_item, items,
               IM_ARRAYSIZE(items));

  // Plot
  ImGui::Spacing();
  ImGui::Text("Live Signal");
  ImGui::Separator();
  static float samples[100];
  for (int n = 0; n < 100; n++)
    samples[n] = sinf(n * 0.2f + ImGui::GetTime() * state_.plot_offset);
  ImGui::PlotLines("Samples", samples, 100, 0, "Sin Wave (realtime)", -1.0f,
                   1.0f, ImVec2(0, 80));

  // Scrolling text area
  ImGui::Spacing();
  ImGui::Text("Log");
  ImGui::Separator();
  ImGui::BeginChild("Scrolling", ImVec2(0, 120), true);
  for (int n = 0; n < 50; n++) {
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "%04d: %s", n,
                       n % 5 == 0   ? "Info message"
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
  if (ImGui::Button("Button 1"))
    fprintf(stdout, "Button 1 clicked\n");
  ImGui::SameLine();
  if (ImGui::Button("Button 2"))
    fprintf(stdout, "Button 2 clicked\n");
  ImGui::SameLine();
  if (ImGui::Button("Button 3"))
    fprintf(stdout, "Button 3 clicked\n");

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
  glfwGetFramebufferSize(window_, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(state_.my_color[0], state_.my_color[1], state_.my_color[2],
               state_.my_color[3]);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  glfwSwapBuffers(window_);
}

#endif
