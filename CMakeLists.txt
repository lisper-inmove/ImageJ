cmake_minimum_required(VERSION 3.16)

project(ImageJ VERSION 0.1 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

include(config/variable.cmake)
include(config/common.cmake)

if(WIN32)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(config/windows/debug.cmake)
  elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(config/windows/release.cmake)
  endif()
elseif(UNIX)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(config/linux/debug.cmake)
  elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(config/linux/release.cmake)
  endif()
endif()

include(config/main.cmake)
