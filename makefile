QT_DIR=$HOME/Qt/6.9.2/gcc_64/lib/cmake/Qt6

debug:
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug && make -j4 && ./ImageJ
