cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "/home/inmove/nvme1/.Qt/6.11.0/gcc_64")

project(ImageJ VERSION 0.1 LANGUAGES CXX)

include(config/qtconfig.cmake)
include(config/main.cmake)

target_sources(ImageJ
  PRIVATE
    include/frames/RightSidebar.h include/frames/RightSidebar.cpp
)

if(WIN32)
  include(config/windows/windows.cmake)
endif()

if(LINUX)
  include(config/linux/linux.cmake)
endif()
