cmake_minimum_required(VERSION 3.16)

project(ImageJ VERSION 0.1 LANGUAGES CXX)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt6 6.2 REQUIRED Qml Quick)  # ← 一定要包含 Qml

include(config/common.cmake)
include(config/main.cmake)
include(config/qmls.cmake)

if(WIN32)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(config/windows/debug.cmake)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(config/windows/release.cmake)
  endif()
endif()
