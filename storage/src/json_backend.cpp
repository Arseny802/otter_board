#include "json_backend.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>

namespace otterboard::storage {

json_backend::json_backend(const std::string& json_path)
    : json_path_(json_path) {
  // TODO: открыть/прочитать JSON-файл
  // TODO: десериализовать данные в внутренние структуры
}

// --- Kanban (колонки и задачи) ---

bool json_backend::load_board(common::KanbanBoard& board) {
  // TODO: прочитать JSON-файл, десериализовать columns и tasks
  return false;
}

bool json_backend::save_board(const common::KanbanBoard& board) {
  // TODO: сериализовать board в JSON, записать в файл
  return false;
}

bool json_backend::add_column(common::Column&& col) {
  // TODO: добавить колонку в JSON-структуру, сохранить
  return false;
}

bool json_backend::update_column(const common::Column& col) {
  // TODO: обновить колонку по id, сохранить
  return false;
}

bool json_backend::remove_column(int64_t col_id) {
  // TODO: удалить колонку и связанные задачи, сохранить
  return false;
}

std::optional<common::Column> json_backend::get_column(int64_t col_id) const {
  // TODO: найти колонку по id в JSON-структуре
  return std::nullopt;
}

bool json_backend::add_task(int64_t col_id, common::Task&& task) {
  // TODO: добавить задачу в указанную колонку, сохранить
  return false;
}

bool json_backend::update_task(const common::Task& task) {
  // TODO: обновить задачу по id, сохранить
  return false;
}

bool json_backend::remove_task(int64_t task_id) {
  // TODO: удалить задачу, сохранить
  return false;
}

bool json_backend::move_task(int64_t task_id, int64_t dest_col_id, int position) {
  // TODO: переместить задачу в другую колонку, обновить порядок, сохранить
  return false;
}

// --- Календарь ---

bool json_backend::load_events(std::vector<common::CalendarEvent>& events) {
  // TODO: десериализовать calendar_events из JSON
  return false;
}

bool json_backend::save_events(const std::vector<common::CalendarEvent>& events) {
  // TODO: сериализовать events, записать в JSON
  return false;
}

bool json_backend::add_event(common::CalendarEvent&& event) {
  // TODO: добавить событие, сохранить
  return false;
}

bool json_backend::update_event(const common::CalendarEvent& event) {
  // TODO: обновить событие, сохранить
  return false;
}

bool json_backend::remove_event(int64_t event_id) {
  // TODO: удалить событие, сохранить
  return false;
}

std::optional<common::CalendarEvent> json_backend::get_event(int64_t event_id) const {
  // TODO: найти событие по id
  return std::nullopt;
}

std::vector<common::CalendarEvent> json_backend::get_events_by_date(const std::string& date) const {
  // TODO: отфильтровать события по дате
  return {};
}

// --- Заметки ---

bool json_backend::load_notes(std::vector<common::Note>& notes) {
  // TODO: десериализовать notes из JSON
  return false;
}

bool json_backend::save_notes(const std::vector<common::Note>& notes) {
  // TODO: сериализовать notes, записать в JSON
  return false;
}

bool json_backend::add_note(common::Note&& note) {
  // TODO: добавить заметку, сохранить
  return false;
}

bool json_backend::update_note(const common::Note& note) {
  // TODO: обновить заметку, сохранить
  return false;
}

bool json_backend::remove_note(int64_t note_id) {
  // TODO: удалить заметку, сохранить
  return false;
}

std::optional<common::Note> json_backend::get_note(int64_t note_id) const {
  // TODO: найти заметку по id
  return std::nullopt;
}

}  // namespace otterboard::storage
