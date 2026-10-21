#pragma once
#include <memory>
#include <string>

#include "storage/backend.h"
#include "storage/i_calendar_storage.h"
#include "storage/i_kanban_storage.h"
#include "storage/i_notes_storage.h"

namespace otterboard::storage {

/// Объединяющий интерфейс Kanban-хранилища.
///
/// Наследует CRUD-операции для колонок/задач, календарных событий и заметок.
/// Реализации: sqlite_backend (SQLite) и json_backend (nlohmann::json).
class kanban_storage : public otterboard::storage::i_kanban_storage,
                       public i_calendar_storage,
                       public i_notes_storage {
public:
  ~kanban_storage() override = default;
};

} // namespace otterboard::storage
