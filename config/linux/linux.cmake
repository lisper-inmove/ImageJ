message(STATUS "Linux build type: ${CMAKE_BUILD_TYPE}")

set(OpenCV_DIR "/home/inmove/.opt/opencv/lib/cmake/opencv4")
# set(GTEST_SOURCE_DIR "/home/inmove/nvme1/SourceCodes/googletest" CACHE PATH "Path to GoogleTest source")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  include(config/linux/debug.cmake)
  include(config/qtest.cmake)
  message(STATUS "Linux Debug build type")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  include(config/linux/release.cmake)
endif()

