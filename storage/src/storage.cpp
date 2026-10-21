#include "storage/storage.hpp"

#include <memory>
#include <stdexcept>

#include "json_backend.h"
#include "sqlite_backend.h"

namespace otterboard::storage {

hare::hlogger_ptr get_logger(const std::string &path) {
  if (!path.empty()) {
    hare::config_ptr cfg =
        std::make_unique<hare::config_default>(PROJECT_NAME, MODULE_NAME);
    cfg->set_log_path(path);
    hare::register_logger(std::move(cfg));
  }
  return log();
}

std::unique_ptr<kanban_storage> open_kanban_storage(backend_type type,
                                                    const std::string &path) {
  switch (type) {
  case backend_type::sqlite:
    return std::make_unique<sqlite_backend>(path);
  case backend_type::json_file:
    return std::make_unique<json_backend>(path);
  }
  return nullptr;
}

} // namespace otterboard::storage