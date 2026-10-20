#pragma once
#include <optional>
#include <string>
#include <vector>

namespace otterboard::storage {

class i_settings_storage {
 public:
  virtual ~i_settings_storage() = default;
};

}  // namespace otterboard::storage
