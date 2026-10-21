#include "settings_window.h"
#include <imgui.h>

namespace otterboard::app {

bool render_settings_window(common::AppSettings &settings, bool &open) {
  bool settings_changed = false;

  ImGui::PushItemWidth(220.0f);

  // ── Тип хранилища ──────────────────────────────────────────────────────────
  ImGui::Text("Тип хранилища");
  ImGui::Separator();

  static int selected_type = 0; // 0 = sqlite, 1 = json
  if (settings.storage_type == common::StorageType::Sqlite)
    selected_type = 0;
  else
    selected_type = 1;

  const char *types[] = {"SQLite (.db)", "JSON (.json)"};
  if (ImGui::Combo("##storage_type", &selected_type, types,
                   IM_ARRAYSIZE(types))) {
    settings.storage_type = static_cast<common::StorageType>(selected_type);
    settings_changed = true;
  }

  // ── Путь к файлу ───────────────────────────────────────────────────────────
  ImGui::Spacing();
  ImGui::Text("Путь к файлу");
  static char path_buf[512] = {};
  strncpy(path_buf, settings.storage_path.c_str(), 511);
  if (ImGui::InputText("##storage_path", path_buf, 511)) {
    settings.storage_path = path_buf;
    settings_changed = true;
  }
  if (settings.storage_path.empty()) {
    settings.storage_path =
        selected_type == 0 ? "otterboard.db" : "otterboard.json";
  }

  // ── Автосохранение ─────────────────────────────────────────────────────────
  ImGui::Spacing();
  ImGui::Text("Автосохранение");
  if (ImGui::Checkbox("##auto_save", &settings.auto_save)) {
    settings_changed = true;
  }

  // ── Цвет фона ──────────────────────────────────────────────────────────────
  ImGui::Spacing();
  ImGui::Text("Цвет фона");
  ImGui::ColorEdit4("##bg_color", settings.background_color.data());

  // ── Размеры окна ───────────────────────────────────────────────────────────
  ImGui::Spacing();
  ImGui::Text("Размеры окна");
  int w = (int)settings.window_width;
  int h = (int)settings.window_height;
  if (ImGui::SliderInt("Ширина", &w, 800, 2560)) {
    settings.window_width = w;
    settings_changed = true;
  }
  if (ImGui::SliderInt("Высота", &h, 600, 1440)) {
    settings.window_height = h;
    settings_changed = true;
  }

  // ── Кнопки ─────────────────────────────────────────────────────────────────
  ImGui::Spacing();
  ImGui::Separator();

  if (ImGui::Button("Применить", ImVec2(-120.0f, 0))) {
    settings_changed = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("Закрыть", ImVec2(-120.0f, 0))) {
    open = false;
  }

  ImGui::PopItemWidth();

  return settings_changed;
}

} // namespace otterboard::app