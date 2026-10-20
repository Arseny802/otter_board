cmake_minimum_required(VERSION 3.23)
project(otterboard VERSION 0.0.1 LANGUAGES CXX)

add_subdirectory(deps)
add_subdirectory(common)
#add_subdirectory(storage)
add_subdirectory(app)

setup_installer(
  TARGETS otterboard.app
  NAME "Divoom Dev"
  DISPLAY_NAME "Divoom Dev ${CMAKE_PROJECT_VERSION}"
  DESCRIPTION "Divoom Settings Server for developer purposes"
  VENDOR "Arseny802"
  CONTACT "arseny802@yandex.ru"
  ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/otterboard.ico"
  NO_SHORTCUT 
)
