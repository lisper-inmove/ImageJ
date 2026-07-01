cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "/home/inmove/nvme1/.Qt/6.11.0/gcc_64")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ImageJ VERSION 0.1 LANGUAGES CXX)

# Copy compile_commands.json to project root for clangd/LSP
add_custom_target(copy_compile_commands ALL
  ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/compile_commands.json"
    "${CMAKE_SOURCE_DIR}/compile_commands.json"
  DEPENDS "${CMAKE_BINARY_DIR}/compile_commands.json"
  COMMENT "Copying compile_commands.json to project root"
)

include(config/qtconfig.cmake)
include(config/main.cmake)

if(WIN32)
  include(config/windows/windows.cmake)
endif()

if(LINUX)
  include(config/linux/linux.cmake)
endif()
