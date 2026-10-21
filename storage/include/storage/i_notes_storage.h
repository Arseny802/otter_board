#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "common/common.hpp"

namespace otterboard::storage {

/// Интерфейс для CRUD-операций с заметками.
class i_notes_storage {
 public:
  virtual ~i_notes_storage() = default;

  /// Загрузить все заметки.
  virtual bool load_notes(std::vector<common::Note>& notes) = 0;

  /// Сохранить все заметки.
  virtual bool save_notes(const std::vector<common::Note>& notes) = 0;

  /// Добавить заметку.
  virtual bool add_note(common::Note&& note) = 0;

  /// Обновить существующую заметку.
  virtual bool update_note(const common::Note& note) = 0;

  /// Удалить заметку по id.
  virtual bool remove_note(int64_t note_id) = 0;

  /// Получить заметку по id.
  virtual std::optional<common::Note> get_note(int64_t note_id) const = 0;
};

}  // namespace otterboard::storage
