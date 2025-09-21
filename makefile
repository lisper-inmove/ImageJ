# Makefile
BUILD_DIR := build
# 你的 Qt 安装根（前缀目录，不是 cmake 目录）
QT_PREFIX ?= $(HOME)/Qt/6.9.2/gcc_64

# 优先用 CMAKE_PREFIX_PATH，更通用
CMAKE_COMMON := -DCMAKE_PREFIX_PATH=$(QT_PREFIX)

all: debug

debug:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. $(CMAKE_COMMON) -DCMAKE_BUILD_TYPE=Debug
	$(MAKE) -C $(BUILD_DIR) -j$(nproc)
	cd ${BUILD_DIR} && ./ImageJ

release:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. $(CMAKE_COMMON) -DCMAKE_BUILD_TYPE=Release
	$(MAKE) -C $(BUILD_DIR) -j$(nproc)

clean:
	rm -rf $(BUILD_DIR)
