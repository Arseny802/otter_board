project(otterboard.app VERSION ${CMAKE_PROJECT_VERSION})

find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)

# Include backends from conan imgui package
include("${CMAKE_BINARY_DIR}/generators/imgui-debug-x86_64-data.cmake")
set(IMGUI_BACKENDS_DIR "${imgui_PACKAGE_FOLDER_DEBUG}/res/bindings")

add_target(${PROJECT_NAME} EXECUTABLE
  SOURCES_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  PCH_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h
  SOURCES
    ${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp
  INCLUDES_PRIVATE
    ${IMGUI_BACKENDS_DIR}
  USES_PUBLIC 
    otterboard.common
    otterboard.storage
    imgui::imgui
    glfw
  TEST_SOURCES_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
  TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../storage/mocs/dbmoc.hpp
  TEST_USES
    imgui::imgui
  TEST_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/../storage/mocs/
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  OUTPUT_NAME "otterboard-${PROJECT_VERSION}" 
)

# Add imgui backends include path to main executable
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_BACKENDS_DIR})
