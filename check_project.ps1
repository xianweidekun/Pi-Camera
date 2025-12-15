# 项目检查脚本
Write-Host "===== CinePI 项目检查 ===="
Write-Host ""

# 检查必要文件是否存在
Write-Host "检查必要文件..."

$files = @(
    "cinepi_raw_recorder.cpp",
    "src/shared/camera_controller.h",
    "src/shared/camera_controller.cpp",
    "src/shared/sdl_helper.h",
    "src/shared/sdl_helper.cpp",
    "CMakeLists.txt",
    "build_recorder.sh"
)

$success = $true

foreach ($file in $files) {
    if (Test-Path $file) {
        Write-Host "✅ $file 存在"
    } else {
        Write-Host "❌ $file 不存在"
        $success = $false
    }
}

Write-Host ""

# 检查关键代码
Write-Host "检查关键代码..."

# 检查 camera_controller.h 中的 FrameBufferMapper
$content = Get-Content "src/shared/camera_controller.h" -ErrorAction SilentlyContinue
if ($content -match "#include <libcamera/framebuffer_mapper.h>") {
    Write-Host "✅ camera_controller.h 包含 FrameBufferMapper 头文件"
} else {
    Write-Host "❌ camera_controller.h 缺少 FrameBufferMapper 头文件"
    $success = $false
}

if ($content -match "std::unique_ptr<libcamera::FrameBufferMapper> mapper_") {
    Write-Host "✅ camera_controller.h 声明了 mapper_ 成员变量"
} else {
    Write-Host "❌ camera_controller.h 缺少 mapper_ 成员变量声明"
    $success = $false
}

# 检查 camera_controller.cpp 中的 mapper_ 初始化
$content = Get-Content "src/shared/camera_controller.cpp" -ErrorAction SilentlyContinue
if ($content -match "mapper_.reset\(new libcamera::FrameBufferMapper\(allocator_.get\(\)\)\)") {
    Write-Host "✅ camera_controller.cpp 正确初始化了 mapper_"
} else {
    Write-Host "❌ camera_controller.cpp 初始化 mapper_ 有误"
    $success = $false
}

Write-Host ""

# 检查 build_recorder.sh
$content = Get-Content "build_recorder.sh" -ErrorAction SilentlyContinue
if ($content -match "src/shared/camera_controller.cpp") {
    Write-Host "✅ build_recorder.sh 包含 camera_controller.cpp"
} else {
    Write-Host "❌ build_recorder.sh 缺少 camera_controller.cpp"
    $success = $false
}

if ($content -match "src/shared/sdl_helper.cpp") {
    Write-Host "✅ build_recorder.sh 包含 sdl_helper.cpp"
} else {
    Write-Host "❌ build_recorder.sh 缺少 sdl_helper.cpp"
    $success = $false
}

Write-Host ""

# 总结
if ($success) {
    Write-Host "===== 检查结果: 成功 ===="
    Write-Host "✅ 所有必要文件和代码结构都正确"
    Write-Host ""
    Write-Host "可以在 Raspberry Pi 上运行以下命令来构建项目："
    Write-Host "  sudo apt install libcamera-dev libsdl2-dev libsdl2-ttf-dev"
    Write-Host "  bash build_recorder.sh"
} else {
    Write-Host "===== 检查结果: 失败 ===="
    Write-Host "❌ 发现一些问题，请检查上面的错误信息"
}
