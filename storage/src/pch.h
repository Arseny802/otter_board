#pragma once

// Standard libraries
#include <gpch.h>

#ifndef PROJECT_NAME
#  define PROJECT_NAME "otterboard"
#endif
#ifndef MODULE_NAME
#  define MODULE_NAME "storage"
#endif
#include <hare/hare.hpp>
namespace otterboard::storage {
MAKE_GETTER(log)
}  // namespace otterboard::storage

// Local files
// #include <sqlite3.h>

// #include <cmrc/cmrc.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
