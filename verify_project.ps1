# 项目验证脚本
Write-Output "===== CinePI 项目验证 ===="
Write-Output ""

# 检查文件是否存在
Write-Output "检查项目文件："

$files = @(
    "cinepi_raw_recorder.cpp",
    "src/shared/camera_controller.h",
    "src/shared/camera_controller.cpp",
    "src/shared/sdl_helper.h",
    "src/shared/sdl_helper.cpp",
    "CMakeLists.txt",
    "build_recorder.sh"
)

foreach ($file in $files) {
    if (Test-Path $file) {
        Write-Output "OK: $file"
    } else {
        Write-Output "ERROR: $file 不存在"
    }
}

Write-Output ""
Write-Output "===== 验证完成 ===="
Write-Output "构建说明："
Write-Output "1. 在 Raspberry Pi 上安装依赖："
Write-Output "   sudo apt install libcamera-dev libsdl2-dev libsdl2-ttf-dev"
Write-Output "2. 运行构建脚本："
Write-Output "   bash build_recorder.sh"
