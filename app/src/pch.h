#pragma once
#include <gpch.h>

#ifndef PROJECT_NAME
#  define PROJECT_NAME "otterboard"
#endif
#ifndef MODULE_NAME
#  define MODULE_NAME "service"
#endif
#include <hare/hare.hpp>
namespace otterboard::service {
MAKE_GETTER(log)
}  // namespace otterboard::service

// External Libraries
