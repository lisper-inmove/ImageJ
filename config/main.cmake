if(WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SPD_ROOT}/include)
    target_include_directories(${PROJECT_NAME} PRIVATE ${YAML_ROOT}/include)
    target_include_directories(${PROJECT_NAME} PRIVATE ${DIR_OPENCV_ROOT}/include)
endif()

# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#   target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
#   target_include_directories(${TEST_NAME} PRIVATE ${SPD_ROOT}/include)
#   target_include_directories(${TEST_NAME} PRIVATE ${YAML_ROOT}/include)
#   target_include_directories(${TEST_NAME} PRIVATE ${DIR_OPENCV_ROOT}/include)
# endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
