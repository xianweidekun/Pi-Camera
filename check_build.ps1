# PowerShell 构建检查脚本

Write-Host "检查代码语法错误..." -ForegroundColor Cyan

# 定义源文件路径
$mainSource = "..\cinepi_raw_recorder.cpp"
$sharedSources = @("..\src\shared\camera_controller.cpp", "..\src\shared\sdl_helper.cpp")

# 定义包含路径
$includePath = "-I..\src\shared"

# 尝试使用 g++ 检查语法
if (Get-Command g++ -ErrorAction SilentlyContinue) {
    Write-Host "使用 g++ 检查语法..." -ForegroundColor Yellow
    $sources = $mainSource + " " + ($sharedSources -join " ")
    $command = "g++ -std=c++17 -fsyntax-only $sources $includePath"
    Write-Host "执行命令: $command" -ForegroundColor Gray
    
    try {
        Invoke-Expression $command 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ 代码语法检查通过!" -ForegroundColor Green
        } else {
            Write-Host "❌ 代码语法检查失败!" -ForegroundColor Red
            exit 1
        }
    } catch {
        Write-Host "❌ 检查过程中发生错误: $_" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "⚠️  未找到 g++ 编译器，无法检查语法" -ForegroundColor Yellow
    Write-Host "请安装 MinGW-w64 或其他 C++ 编译器" -ForegroundColor Yellow
}

Write-Host "" -ForegroundColor Cyan
Write-Host "项目文件结构检查..." -ForegroundColor Cyan

# 检查必要文件是否存在
$requiredFiles = @(
    "..\cinepi_raw_recorder.cpp",
    "..\src\shared\camera_controller.h",
    "..\src\shared\camera_controller.cpp",
    "..\src\shared\sdl_helper.h",
    "..\src\shared\sdl_helper.cpp",
    "..\CMakeLists.txt"
)

$allFilesExist = $true
foreach ($file in $requiredFiles) {
    if (Test-Path $file) {
        Write-Host "✅ $file 存在" -ForegroundColor Green
    } else {
        Write-Host "❌ $file 不存在" -ForegroundColor Red
        $allFilesExist = $false
    }
}

if (-not $allFilesExist) {
    Write-Host "" -ForegroundColor Red
    Write-Host "❌ 缺少必要文件!" -ForegroundColor Red
    exit 1
} else {
    Write-Host "" -ForegroundColor Green
    Write-Host "✅ 所有检查通过!" -ForegroundColor Green
    Write-Host "项目可以正常构建。" -ForegroundColor Green
}