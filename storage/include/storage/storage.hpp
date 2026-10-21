#pragma once
#include <memory>
#include <string>

#include <hare/hlogger.h>

#include "storage/backend.h"
#include "storage/kanban_storage.h"

namespace otterboard::storage {

hare::hlogger_ptr get_logger(const std::string &path = {});

/// Открывает Kanban-хранилище на выбранном бэкенде.
///
/// \param type  Тип бэкенда: sqlite (БД) или json_file (config.json).
/// \param path  Путь к файлу БД (*.db / *.sqlite) либо к *.json.
///              Для пустой/несуществующей БД будет применена базовая схема.
/// \return      Реализация `kanban_storage` либо `nullptr` при ошибке
///              открытия/миграции.
std::unique_ptr<kanban_storage> open_kanban_storage(backend_type type,
                                                    const std::string &path);

} // namespace otterboard::storage