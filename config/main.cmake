# Core library
add_library(imagej_core STATIC ${PROJECT_SOURCES} ${PROJECT_HEADERS})
target_link_libraries(imagej_core PRIVATE Qt${QT_VERSION_MAJOR}::Widgets opencv_core opencv_imgproc opencv_imgcodecs)
target_include_directories(imagej_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(imagej_core PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)

# Main executable
qt_add_executable(${PROJECT_NAME}
  MANUAL_FINALIZATION
  ${PROJECT_HEADERS}
  ${PROJECT_FORMS}
  ${PROJECT_RESOURCES}
  src/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets imagej_core)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
qt_finalize_executable(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Unit tests
enable_testing()

file(GLOB_RECURSE TEST_SOURCES
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/core/*.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/widgets/*.cc"
)

if(Qt6Test_FOUND)
  find_package(GTest)
  if(GTest_FOUND)
    add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})
    set_target_properties(${PROJECT_NAME}_tests PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE Qt6::Test Qt6::Widgets GTest::gtest GTest::gtest_main imagej_core)
    target_include_directories(${PROJECT_NAME}_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
    add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)
  else()
    message(WARNING "GTest not found - test target will not be created")
  endif()
endif()
