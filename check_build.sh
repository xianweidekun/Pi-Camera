#!/bin/bash

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
pkg-config --exists SDL2_ttf > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "错误: 未安装SDL2_ttf库"
    echo "请先安装: sudo apt install libsdl2-ttf-dev"
    exit 1
fi

# 检查libcamera
pkg-config --exists libcamera > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "错误: 未安装libcamera库"
    echo "请先安装: sudo apt install libcamera-dev libcamera-apps"
    exit 1
fi

echo "所有依赖检查通过!"
echo ""

# 检查代码语法错误
echo "检查代码语法错误..."
g++ -std=c++17 -fsyntax-only ../cinepi_raw_recorder.cpp ../src/shared/camera_controller.cpp ../src/shared/sdl_helper.cpp -I../src/shared $(pkg-config --cflags libcamera sdl2 SDL2_ttf)

if [ $? -eq 0 ]; then
    echo "代码语法检查通过!"
    echo "项目可以正常构建。"
else
    echo "代码语法检查失败!"
    exit 1
fi