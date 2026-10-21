#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "common/common.hpp"

namespace otterboard::storage {

/// Интерфейс для CRUD-операций с календарными событиями.
class i_calendar_storage {
 public:
  virtual ~i_calendar_storage() = default;

  /// Загрузить все события календаря.
  virtual bool load_events(std::vector<common::CalendarEvent>& events) = 0;

  /// Сохранить все события календаря.
  virtual bool save_events(const std::vector<common::CalendarEvent>& events) = 0;

  /// Добавить событие.
  virtual bool add_event(common::CalendarEvent&& event) = 0;

  /// Обновить существующее событие.
  virtual bool update_event(const common::CalendarEvent& event) = 0;

  /// Удалить событие по id.
  virtual bool remove_event(int64_t event_id) = 0;

  /// Получить событие по id.
  virtual std::optional<common::CalendarEvent> get_event(int64_t event_id) const = 0;

  /// Получить события, отфильтрованные по дате (YYYY-MM-DD).
  virtual std::vector<common::CalendarEvent> get_events_by_date(const std::string& date) const = 0;
};

}  // namespace otterboard::storage
