cmake_minimum_required(VERSION 3.21)
project(todo-rust-qt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  add_compile_options(/utf-8)
endif()

if(NOT DEFINED CMAKE_PREFIX_PATH OR CMAKE_PREFIX_PATH STREQUAL "")
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/../.qt/6.8.2/msvc2022_64")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

include(FetchContent)
FetchContent_Declare(
  Corrosion
  GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
  GIT_TAG v0.5.1
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Corrosion)

corrosion_import_crate(MANIFEST_PATH rust/Cargo.toml CRATES todo_core)

qt_standard_project_setup()

add_executable(todo-rust-qt WIN32
  cpp/main.cpp
  cpp/main_window.cpp
)
target_include_directories(todo-rust-qt PRIVATE cpp)
target_link_libraries(todo-rust-qt PRIVATE Qt6::Widgets todo_core)

if(MSVC)
  set_target_properties(todo-rust-qt PROPERTIES OUTPUT_NAME "todo-rust-qt")
endif()

find_program(WINDEPLOYQT windeployqt
  HINTS "${Qt6_DIR}/../../../bin" "${CMAKE_PREFIX_PATH}/bin")
if(WINDEPLOYQT)
  add_custom_command(TARGET todo-rust-qt POST_BUILD
    COMMAND "${WINDEPLOYQT}" "$<TARGET_FILE:todo-rust-qt>"
    COMMENT "windeployqt todo-rust-qt")
endif()
