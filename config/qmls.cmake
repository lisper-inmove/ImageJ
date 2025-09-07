target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Quick)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

set(QML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qmls)

# QML/JS（会参与 qmlcache）
file(GLOB_RECURSE APP_QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS
    ${QML_DIR}/*.qml
    ${QML_DIR}/*.js
    ${QML_DIR}/*.mjs
    ${QML_DIR}/*.qmltypes
)

# 其它资源（图片/字体/数据等）
file(GLOB_RECURSE APP_QML_ASSETS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS
    ${QML_DIR}/*.png
    ${QML_DIR}/*.jpg
    ${QML_DIR}/*.svg
    ${QML_DIR}/*.gif
    ${QML_DIR}/*.ttf
    ${QML_DIR}/*.otf
    ${QML_DIR}/*.json
)

# ---- 声明一个 QML 模块，把上面搜到的都塞进去 ----
# URI 自定义（建议与你项目名一致），加载时用 engine.loadFromModule(URI, "Main")
qt_add_qml_module(${PROJECT_NAME}
    URI Main
    VERSION 1.0
    QML_FILES ${APP_QML_FILES}
    RESOURCES ${APP_QML_ASSETS}
    # 可选：
    # RESOURCE_PREFIX "/"        # 想用 qrc:/ 路径访问时可以改前缀
)

# （可选）启用 qmlcache（通常默认开）
set_property(TARGET ${PROJECT_NAME} PROPERTY QT_QMLCACHEGEN_ENABLED ON)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Quick)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
