# 简单的文件检查脚本
Write-Host "===== CinePI 项目文件检查 ===="
Write-Host ""

# 检查必要文件是否存在
$filesToCheck = @(
    "cinepi_raw_recorder.cpp",
    "src/shared/camera_controller.h",
    "src/shared/camera_controller.cpp",
    "src/shared/sdl_helper.h",
    "src/shared/sdl_helper.cpp",
    "CMakeLists.txt",
    "build_recorder.sh"
)

Write-Host "检查项目文件是否存在："
foreach ($file in $filesToCheck) {
    if (Test-Path $file) {
        Write-Host "✓ $file"
    } else {
        Write-Host "✗ $file"
    }
}

Write-Host ""
Write-Host "===== 检查完成 ===="
Write-Host "可以在 Raspberry Pi 上运行以下命令来构建项目："
Write-Host "sudo apt install libcamera-dev libsdl2-dev libsdl2-ttf-dev"
Write-Host "bash build_recorder.sh"
