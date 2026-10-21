#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "storage/kanban_storage.h"

namespace otterboard::storage {

/// Заглушка SQLite-бэкенда.
///
/// Реализует все методы объединяющего интерфейса, но пока возвращает
/// false/пустые значения — при интеграции с SQLite здесь будут SQL-запросы.
class sqlite_backend : public kanban_storage {
public:
  explicit sqlite_backend(const std::string &db_path);
  ~sqlite_backend() override = default;

  // --- Kanban (колонки и задачи) ---

  bool load_board(common::KanbanBoard &board) override;
  bool save_board(const common::KanbanBoard &board) override;
  bool add_column(common::Column &&col) override;
  bool update_column(const common::Column &col) override;
  bool remove_column(int64_t col_id) override;
  std::optional<common::Column> get_column(int64_t col_id) const override;
  bool add_task(int64_t col_id, common::Task &&task) override;
  bool update_task(const common::Task &task) override;
  bool remove_task(int64_t task_id) override;
  bool move_task(int64_t task_id, int64_t dest_col_id, int position) override;

  // --- Календарь ---

  bool load_events(std::vector<common::CalendarEvent> &events) override;
  bool save_events(const std::vector<common::CalendarEvent> &events) override;
  bool add_event(common::CalendarEvent &&event) override;
  bool update_event(const common::CalendarEvent &event) override;
  bool remove_event(int64_t event_id) override;
  std::optional<common::CalendarEvent>
  get_event(int64_t event_id) const override;
  std::vector<common::CalendarEvent>
  get_events_by_date(const std::string &date) const override;

  // --- Заметки ---

  bool load_notes(std::vector<common::Note> &notes) override;
  bool save_notes(const std::vector<common::Note> &notes) override;
  bool add_note(common::Note &&note) override;
  bool update_note(const common::Note &note) override;
  bool remove_note(int64_t note_id) override;
  std::optional<common::Note> get_note(int64_t note_id) const override;

private:
  std::string db_path_;
};

} // namespace otterboard::storage
