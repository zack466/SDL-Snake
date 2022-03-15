cmake_minimum_required(VERSION 3.10)

project(SDL-Snake VERSION 0.1)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2_TTF REQUIRED sdl2_ttf)
include_directories(${SDL2_TTF_include_dirs})
link_directories(${SDL2_TTF_library_dirs})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(Bitset src/bitset.c)
add_library(Ringbuf src/ringbuf.c)

add_executable(Snake src/snake.c)
target_link_libraries(Snake Bitset Ringbuf ${SDL2_TTF_LIBRARIES} ${SDL2_LIBRARIES})

add_custom_target(
  run
  COMMAND Snake
  DEPENDS Snake
)
