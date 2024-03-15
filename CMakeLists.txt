cmake_minimum_required(VERSION 3.5)
project(mqec VERSION 0.1 LANGUAGES C)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

add_executable(mqec
  src/main.c
  src/die.c
  src/emulator.c
  src/program.c
  src/commands.c
)

set_target_properties(mqec PROPERTIES C_STANDARD 99)
set_target_properties(mqec PROPERTIES C_EXTENSIONS FALSE)

target_include_directories(mqec PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
