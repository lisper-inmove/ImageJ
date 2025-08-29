message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

set(OPENCV_ROOT "D:/code/cpplibs/opencv-debug/")

include(config/windows/googletest.cmake)
