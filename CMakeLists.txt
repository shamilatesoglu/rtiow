cmake_minimum_required(VERSION 3.15)

project(rtiow)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
add_subdirectory(third_party/raylib)
add_library(raygui INTERFACE)
target_include_directories(raygui INTERFACE third_party/raygui/src)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cc")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/*.h")

add_executable(rtiow ${SOURCES} ${HEADERS})

target_precompile_headers(rtiow PRIVATE src/pch.h)
target_link_libraries(rtiow raylib raygui)
