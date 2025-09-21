message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# ---------------- Spdlog ---------------------------
# target_link_libraries(${PROJECT_NAME} PRIVATE "${SPD_ROOT}/spdlogd.lib")
find_package(spdlog REQUIRED)

# ---------------- YAML-cpp ---------------------------
# target_link_libraries(${PROJECT_NAME} PRIVATE "${YAML_ROOT}/yaml-cppd.lib")
find_package(yaml-cpp REQUIRED)

find_package(OpenCV REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})

# include(config/linux/googletest.cmake)

add_compile_options(/Zi)
add_link_options(/DEBUG)
