#!/bin/bash

# CinePI预览应用编译脚本

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

# 编译预览应用
echo "编译cinepi_preview应用..."
g++ -std=c++17 ../cinepi_preview.cpp -o cinepi_preview \
    $(pkg-config --cflags --libs cinepi) \
    $(pkg-config --cflags --libs sdl2) \
    $(pkg-config --cflags --libs SDL2_ttf)

if [ $? -eq 0 ]; then
    echo "编译成功!"
    echo ""
    echo "运行预览应用: ./cinepi_preview"
    echo ""
    echo "使用说明:"
    echo "  空格键: 开始/停止预览"
    echo "  方向键上/下: 调整曝光补偿"
    echo "  方向键左/右: 调整ISO"
    echo "  W键: 循环切换白平衡"
    echo "  ESC键: 退出应用"
else
    echo "编译失败!"
    exit 1
fi

# 清理临时文件
# rm -rf build