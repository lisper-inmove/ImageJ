enable_testing()

# add_definitions(-DUNIT_TEST)

set(TEST_NAME "ImageJ_Test")

file(GLOB_RECURSE SRC_LIST CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
file(GLOB_RECURSE TEST_SRC_LIST CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cc)

# 将源代码添加到此项目的可执行文件。
# add_executable (${TEST_NAME} tests/test.cpp ${SRC_LIST} ${TEST_SRC_LIST})

##################################
# For Qt
qt_add_executable(${TEST_NAME}
    MANUAL_FINALIZATION
    ${PROJECT_HEADERS}
    ${PROJECT_SOURCES}
    ${PROJECT_FORMS}
    ${PROJECT_RESOURCES}
    ${TEST_SRC_LIST}
    tests/test.cpp
)

target_link_libraries(${TEST_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

include(GNUInstallDirs)
install(TARGETS ${TEST_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
qt_finalize_executable(${TEST_NAME})

target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

# ========== 链接库 ==========
target_link_libraries(${TEST_NAME} 
    PRIVATE
    gtest_main      # GoogleTest 主函数
    gtest           # GoogleTest 核心库
    ${OpenCV_LIBS}  # OpenCV 库
    Qt6::Core 
    Qt6::Widgets
    Qt6::Test
)

# ========== 关键：添加测试到 CTest ==========
# 添加测试，这样 ctest 就能发现它
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

# 可选：设置测试属性
# 设置超时时间（秒），防止测试卡死
set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT 30)

message(STATUS "Add ${TEST_NAME} test success")
