#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "common/common.hpp"

namespace otterboard::storage {

/// Интерфейс для CRUD-операций с Kanban-доской (колонки и задачи).
class i_kanban_storage {
 public:
  virtual ~i_kanban_storage() = default;

  /// Загрузить всю доску из хранилища.
  virtual bool load_board(common::KanbanBoard& board) = 0;

  /// Сохранить всю доску в хранилище.
  virtual bool save_board(const common::KanbanBoard& board) = 0;

  /// Добавить новую колонку. Возвращает true при успехе.
  virtual bool add_column(common::Column&& col) = 0;

  /// Обновить существующую колонку (по id).
  virtual bool update_column(const common::Column& col) = 0;

  /// Удалить колонку по id.
  virtual bool remove_column(int64_t col_id) = 0;

  /// Получить колонку по id (возвращает пустой optional, если не найдена).
  virtual std::optional<common::Column> get_column(int64_t col_id) const = 0;

  /// Добавить задачу в указанную колонку.
  virtual bool add_task(int64_t col_id, common::Task&& task) = 0;

  /// Обновить существующую задачу (по id).
  virtual bool update_task(const common::Task& task) = 0;

  /// Удалить задачу по id.
  virtual bool remove_task(int64_t task_id) = 0;

  /// Переместить задачу в другую колонку и/или изменить порядок.
  virtual bool move_task(int64_t task_id, int64_t dest_col_id, int position) = 0;
};

}  // namespace otterboard::storage
