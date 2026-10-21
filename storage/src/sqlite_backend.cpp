#include "sqlite_backend.h"

#include <nlohmann/json.hpp>

namespace otterboard::storage {

sqlite_backend::sqlite_backend(const std::string& db_path)
    : db_path_(db_path) {
  // TODO: открыть SQLite-базу данных (sqlite3_open / C++ API)
  // TODO: выполнить DDL-скрипты, если таблицы не существуют
}

// --- Kanban (колонки и задачи) ---

bool sqlite_backend::load_board(common::KanbanBoard& board) {
  // TODO: SELECT * FROM columns; SELECT * FROM tasks WHERE col_id = ?
  return false;
}

bool sqlite_backend::save_board(const common::KanbanBoard& board) {
  // TODO: INSERT/REPLACE INTO columns, tasks
  return false;
}

bool sqlite_backend::add_column(common::Column&& col) {
  // TODO: INSERT INTO columns (title) VALUES (?)
  return false;
}

bool sqlite_backend::update_column(const common::Column& col) {
  // TODO: UPDATE columns SET title = ? WHERE id = ?
  return false;
}

bool sqlite_backend::remove_column(int64_t col_id) {
  // TODO: DELETE FROM tasks WHERE col_id = ?; DELETE FROM columns WHERE id = ?
  return false;
}

std::optional<common::Column> sqlite_backend::get_column(int64_t col_id) const {
  // TODO: SELECT * FROM columns WHERE id = ?
  return std::nullopt;
}

bool sqlite_backend::add_task(int64_t col_id, common::Task&& task) {
  // TODO: INSERT INTO tasks (col_id, title, description, date_due, priority, color, position)
  return false;
}

bool sqlite_backend::update_task(const common::Task& task) {
  // TODO: UPDATE tasks SET ... WHERE id = ?
  return false;
}

bool sqlite_backend::remove_task(int64_t task_id) {
  // TODO: DELETE FROM tasks WHERE id = ?
  return false;
}

bool sqlite_backend::move_task(int64_t task_id, int64_t dest_col_id, int position) {
  // TODO: UPDATE tasks SET col_id = ?, position = ? WHERE id = ?
  return false;
}

// --- Календарь ---

bool sqlite_backend::load_events(std::vector<common::CalendarEvent>& events) {
  // TODO: SELECT * FROM calendar_events
  return false;
}

bool sqlite_backend::save_events(const std::vector<common::CalendarEvent>& events) {
  // TODO: INSERT/REPLACE INTO calendar_events
  return false;
}

bool sqlite_backend::add_event(common::CalendarEvent&& event) {
  // TODO: INSERT INTO calendar_events
  return false;
}

bool sqlite_backend::update_event(const common::CalendarEvent& event) {
  // TODO: UPDATE calendar_events SET ...
  return false;
}

bool sqlite_backend::remove_event(int64_t event_id) {
  // TODO: DELETE FROM calendar_events WHERE id = ?
  return false;
}

std::optional<common::CalendarEvent> sqlite_backend::get_event(int64_t event_id) const {
  // TODO: SELECT * FROM calendar_events WHERE id = ?
  return std::nullopt;
}

std::vector<common::CalendarEvent> sqlite_backend::get_events_by_date(const std::string& date) const {
  // TODO: SELECT * FROM calendar_events WHERE date = ?
  return {};
}

// --- Заметки ---

bool sqlite_backend::load_notes(std::vector<common::Note>& notes) {
  // TODO: SELECT * FROM notes
  return false;
}

bool sqlite_backend::save_notes(const std::vector<common::Note>& notes) {
  // TODO: INSERT/REPLACE INTO notes
  return false;
}

bool sqlite_backend::add_note(common::Note&& note) {
  // TODO: INSERT INTO notes
  return false;
}

bool sqlite_backend::update_note(const common::Note& note) {
  // TODO: UPDATE notes SET ...
  return false;
}

bool sqlite_backend::remove_note(int64_t note_id) {
  // TODO: DELETE FROM notes WHERE id = ?
  return false;
}

std::optional<common::Note> sqlite_backend::get_note(int64_t note_id) const {
  // TODO: SELECT * FROM notes WHERE id = ?
  return std::nullopt;
}

}  // namespace otterboard::storage
