# Linux 下编译运行

    export Qt6_DIR=$HOME/Qt/6.9.2/gcc_64/lib/cmake/Qt6 or
    export QT_DIR=$HOME/Qt/6.9.2/gcc_64/lib/cmake/Qt6

    mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug
