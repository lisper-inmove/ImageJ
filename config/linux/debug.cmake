message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

set(OPENCV_ROOT "/home/inmove/code/cpplibs/opencv-debug/")

# ---------------- Spdlog ---------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE "${SPD_ROOT}/spdlogd.lib")

# ---------------- YAML-cpp ---------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE "${YAML_ROOT}/yaml-cppd.lib")

include(config/linux/googletest.cmake)

add_compile_options(/Zi)
add_link_options(/DEBUG)
