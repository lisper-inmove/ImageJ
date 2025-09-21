message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# ---------------- Spdlog ---------------------------
find_package(spdlog REQUIRED)

# ---------------- yaml-cpp -------------------------
find_package(yaml-cpp REQUIRED)

# ---------------- OpenCV ---------------------------
find_package(OpenCV REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})

include(config/linux/googletest.cmake)

if (MSVC)
  add_compile_options(/Zi)
  add_link_options(/DEBUG)
endif()
