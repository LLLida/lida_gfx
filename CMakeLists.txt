cmake_minimum_required(VERSION 3.0)
project(LIDA_GFX VERSION 0.1)

add_library(lida_gfx STATIC "lida_gfx_vulkan.c" "lida_gfx.h")

target_include_directories(lida_gfx PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

set_target_properties(lida_gfx PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS OFF)

# find the SDL2 library
find_package(SDL2 REQUIRED)
target_link_libraries(lida_gfx SDL2::SDL2)

set(ENABLE_ASAN 1)
set(ENABLE_STATIC_ANALYZER 0)

# enable all warnings from compiler
target_compile_options(lida_gfx PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>                         # MSVC
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Wno-unused-function> # g++ or clang
)

if (ENABLE_ASAN)
  target_compile_options(lida_gfx PRIVATE
    -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(lida_gfx PUBLIC
    -fsanitize=address -fno-omit-frame-pointer -lrt)
endif ()

if (ENABLE_STATIC_ANALYZER)
  target_compile_options(lida_gfx PRIVATE
    -fanalyzer)
endif ()

option(LIDA_GFX_BUILD_SAMPLES "Enable building of samples." OFF)

if (${LIDA_GFX_BUILD_SAMPLES})
  add_subdirectory(samples)
endif ()
