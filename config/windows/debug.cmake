message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# ---------------- Spdlog ---------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE "${SPD_ROOT}/spdlogd.lib")

# ---------------- YAML-cpp -------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE "${YAML_ROOT}/yaml-cppd.lib")

# ---------------- OpenCV ---------------------------
file(GLOB OPENCV_LIBS "${DIR_OPENCV_ROOT}/lib/*.lib")

target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENCV_LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE ${DIR_OPENCV_ROOT}/include)

include(config/windows/googletest.cmake)

add_compile_options(/Zi)
add_link_options(/DEBUG)
