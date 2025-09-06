message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

set(OPENCV_ROOT "D:/code/cpplibs/opencv-debug/")

# ---------------- Spdlog ---------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE "${SPD_ROOT}/spdlog.lib")

# ---------------- YAML cpp ---------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE "${YAML_ROOT}/yaml-cpp.lib")