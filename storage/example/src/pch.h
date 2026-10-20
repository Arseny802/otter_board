#pragma once
#include <gpch.h>

#ifndef PROJECT_NAME
#  define PROJECT_NAME "otterboard"
#endif
#ifndef MODULE_NAME
#  define MODULE_NAME "storage.example"
#endif
#include <hare/hare.hpp>
namespace otterboard::storage::example {
MAKE_GETTER(log)
}  // namespace otterboard::storage::example
