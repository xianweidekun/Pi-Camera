#!/bin/bash

# CinePI项目统一编译脚本

# 检查是否安装了必要的依赖
echo "检查必要的依赖..."

# 检查gcc
gcc --version > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "错误: 未安装gcc编译器"
    echo "请先安装: sudo apt install build-essential"
    exit 1
fi

# 检查cmake
cmake --version > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "错误: 未安装cmake"
    echo "请先安装: sudo apt install cmake"
    exit 1
fi

# 检查SDL2
sdl2-config --version > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "错误: 未安装SDL2库"
    echo "请先安装: sudo apt install libsdl2-dev"
    exit 1
fi

# 检查SDL2_ttf
sdl2-config --cflags | grep SDL2_ttf > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "错误: 未安装SDL2_ttf库"
    echo "请先安装: sudo apt install libsdl2-ttf-dev"
    exit 1
fi

# 检查CinePI SDK
pkg-config --exists cinepi > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "错误: 未安装CinePI SDK"
    echo "请先安装CinePI SDK"
    exit 1
fi

echo "所有依赖检查通过!"
echo ""

# 创建构建目录
mkdir -p build
echo "切换到构建目录..."
cd build

# 编译共享模块
echo "编译共享模块..."

# 确保共享模块目录存在
mkdir -p ../src/shared

# 编译SDL Helper
g++ -std=c++17 -c ../src/shared/sdl_helper.cpp -o sdl_helper.o \
    $(pkg-config --cflags --libs sdl2) \
    $(pkg-config --cflags --libs SDL2_ttf)

if [ $? -ne 0 ]; then
    echo "编译SDL Helper失败!"
    exit 1
fi

# 编译摄像头控制器
g++ -std=c++17 -c ../src/shared/camera_controller.cpp -o camera_controller.o \
    $(pkg-config --cflags --libs cinepi)

if [ $? -ne 0 ]; then
    echo "编译摄像头控制器失败!"
    exit 1
fi

# 创建共享库
ar rcs libcinepi_shared.a sdl_helper.o camera_controller.o

if [ $? -ne 0 ]; then
    echo "创建共享库失败!"
    exit 1
fi

echo "共享模块编译成功!"
echo ""

# 编译预览应用
echo "编译cinepi_preview应用..."
g++ -std=c++17 ../cinepi_preview.cpp -o cinepi_preview \
    -L. -lcinepi_shared \
    $(pkg-config --cflags --libs cinepi) \
    $(pkg-config --cflags --libs sdl2) \
    $(pkg-config --cflags --libs SDL2_ttf)

if [ $? -eq 0 ]; then
    echo "预览应用编译成功!"
else
    echo "预览应用编译失败!"
    exit 1
fi

# 编译RAW录制应用
echo "编译cinepi_raw_recorder应用..."
g++ -std=c++17 ../cinepi_raw_recorder.cpp -o cinepi_raw_recorder \
    -L. -lcinepi_shared \
    $(pkg-config --cflags --libs cinepi) \
    $(pkg-config --cflags --libs sdl2) \
    $(pkg-config --cflags --libs SDL2_ttf)

if [ $? -eq 0 ]; then
    echo "RAW录制应用编译成功!"
else
    echo "RAW录制应用编译失败!"
    exit 1
fi

echo ""
echo "所有应用编译成功!"
echo ""
echo "运行预览应用: ./cinepi_preview"
echo "运行RAW录制应用: ./cinepi_raw_recorder [录制目录]"
echo "默认录制目录: /home/pi/cinepi_recordings"
echo ""
echo "使用说明:"
echo "  空格键: 开始/停止预览/录制"
echo "  方向键上/下: 调整曝光补偿"
echo "  方向键左/右: 调整ISO"
echo "  W键: 循环切换白平衡"
echo "  ESC键: 退出应用"

# 复制可执行文件到项目根目录
cp cinepi_preview ..
cp cinepi_raw_recorder ..
echo ""
echo "可执行文件已复制到项目根目录"