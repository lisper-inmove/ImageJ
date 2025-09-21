enable_testing()

# add_definitions(-DUNIT_TEST)

file(GLOB_RECURSE TEST_SOURCES
     CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
)

qt_add_executable(${TEST_NAME}
    MANUAL_FINALIZATION
    ${TEST_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_SOURCES}
    ${PROJECT_FORMS}
    ${PROJECT_RESOURCES}
    tests/test.cpp
)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# --------------------- googletest -----------------------------
target_link_libraries(${TEST_NAME} PRIVATE GTest::gtest GTest::gtest_main pthread)

# --------------------- OpenCV ---------------------------------
target_link_libraries(${TEST_NAME} PRIVATE ${OpenCV_LIBS})
target_include_directories(${TEST_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(${TEST_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Test)

include(GNUInstallDirs)
install(TARGETS ${TEST_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
qt_finalize_executable(${TEST_NAME})
