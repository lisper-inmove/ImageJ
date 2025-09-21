# 从 https://opencv.org/releases/ 下载 opencv并安装，假设安装 在D:\code\cpplibs\opencv目录下
# 找到 D:\code\cpplibs\opencv\build\x64\vc16\bin ，将其添加到 PATH中（dll文件）

# CMake 文件中增加如下配置

# set(OpenCV_DIR "D:/code/cpplibs/opencv/build")
# find_package(OpenCV REQUIRED)

# target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
# target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})