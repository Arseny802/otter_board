#pragma once

#include <array>
#include <string>

#include <common/common.hpp>

namespace otterboard::app {

/// Рендерит окно настроек хранения.
///
/// \param settings  Ссылка на AppSettings для чтения/записи.
/// \param open      Флаг "окно открыто". Если пользователь нажал "Закрыть" —
///                  будет установлен в false.
///
/// \return true, если настройки изменены и требуется пересоздать хранилище.
bool render_settings_window(common::AppSettings &settings, bool &open);

} // namespace otterboard::app