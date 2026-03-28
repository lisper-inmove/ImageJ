cmake_minimum_required(VERSION 3.16)

project(ImageJ VERSION 0.1 LANGUAGES CXX)

include(config/qtconfig.cmake)
include(config/main.cmake)

if(WIN32)
  include(config/windows/windows.cmake)
endif()

if(LINUX)
  include(config/linux/linux.cmake)
endif()
