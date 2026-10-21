#pragma once
#include "version.h"
#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace otterboard::common {

// ── Утилиты ────────────────────────────────────────────────────────────────

inline static int64_t next_id() {
  static int64_t counter = 0;
  return counter++;
}

// ── Тип хранилища ──────────────────────────────────────────────────────────

enum class StorageType : int {
  Sqlite = 0,
  JsonFile = 1,
};

inline const char *storage_type_to_string(StorageType t) {
  switch (t) {
  case StorageType::Sqlite:
    return "sqlite";
  case StorageType::JsonFile:
    return "json_file";
  }
  return "unknown";
}

// ── Настройки приложения ───────────────────────────────────────────────────

struct AppSettings {
  StorageType storage_type = StorageType::Sqlite;
  std::string storage_path = "otterboard.db";
  bool auto_save = true;
  int64_t window_width = 1280;
  int64_t window_height = 720;
  std::array<float, 4> background_color = {0.12f, 0.12f, 0.16f, 1.0f};
};

// ── Приоритет задачи ───────────────────────────────────────────────────────

enum class TaskPriority : int {
  Low = 0,
  Medium = 1,
  High = 2,
};

inline const char *task_priority_to_string(TaskPriority p) {
  switch (p) {
  case TaskPriority::Low:
    return "Низкий";
  case TaskPriority::Medium:
    return "Средний";
  case TaskPriority::High:
    return "Высокий";
  }
  return "Неизвестный";
}

// ── Задача (карточка) ──────────────────────────────────────────────────────

struct Task {
  int64_t id = next_id();
  std::string title = "";
  std::string description = "";
  std::string date_due = ""; // YYYY-MM-DD
  TaskPriority priority = TaskPriority::Medium;

  // Дополнительные поля для GUI
  std::array<float, 4> color = {0.8f, 0.8f, 0.8f, 1.0f};
  std::vector<std::string> tags;
  int position = 0;
};

// ── Колонка доски ──────────────────────────────────────────────────────────

struct Column {
  int64_t id = next_id();
  std::string title = "";
  std::vector<Task> tasks;
};

// ── Календарное событие ────────────────────────────────────────────────────

struct CalendarEvent {
  int64_t id = next_id();
  std::string title = "";
  std::string description = "";
  std::string date = ""; // YYYY-MM-DD

  // Дополнительные поля для GUI
  std::array<float, 4> color = {0.3f, 0.6f, 1.0f, 1.0f};
};

// ── Заметка ────────────────────────────────────────────────────────────────

struct Note {
  int64_t id = next_id();
  std::string title = "";
  std::string content = "";

  // Дополнительные поля для GUI
  std::array<float, 4> color = {1.0f, 1.0f, 0.8f, 1.0f};
};

// ── Состояние приложения ───────────────────────────────────────────────────

struct KanbanBoard {
  std::vector<Column> columns;
};

struct MainState {
  KanbanBoard board;
  std::vector<CalendarEvent> calendar_events;
  std::vector<Note> notes;
  AppSettings app_settings;
};

} // namespace otterboard::common
