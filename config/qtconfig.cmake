set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(TS_FILES ImageJ_en_AS.ts)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt6 COMPONENTS Test)

# 输出找到的 Qt 组件信息（用于调试）
message(STATUS "Qt6 found: ${Qt6_FOUND}")
message(STATUS "Qt6 Core: ${Qt6Core_FOUND}")
message(STATUS "Qt6 Widgets: ${Qt6Widgets_FOUND}")
message(STATUS "Qt6 Test: ${Qt6Test_FOUND}")

file(GLOB_RECURSE PROJECT_HEADERS
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/core/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/widgets/*.h"
)

file(GLOB_RECURSE PROJECT_SOURCES
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/*.cc"
)

file(GLOB_RECURSE PROJECT_FORMS
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/uis/*.ui"
)
file(GLOB_RECURSE PROJECT_RESOURCES
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/qrcs/*.qrc"
)

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
