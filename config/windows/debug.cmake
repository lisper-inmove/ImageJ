message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

set(DIR_OPENCV_ROOT "D:/code/cpplibs/opencv/")
set(OpenCV_DIR "D:/code/cpplibs/opencv/build")
find_package(OpenCV REQUIRED)

# ---------------- Spdlog ---------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE "${SPD_ROOT}/spdlogd.lib")

# ---------------- YAML-cpp -------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE "${YAML_ROOT}/yaml-cppd.lib")

# ---------------- OpenCV ---------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})

include(config/windows/googletest.cmake)

add_compile_options(/Zi)
add_link_options(/DEBUG)
