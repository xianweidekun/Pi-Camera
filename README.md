# CinePI RAW视频录制系统 - 树莓派5 + IMX477方案

基于树莓派5和IMX477摄像头模块的CinePI RAW视频录制系统，支持外接显示屏实时预览、键盘鼠标参数控制和RAW视频录制功能。

## 项目概述

本项目基于CinePI SDK开发，实现了一个完整的RAW视频录制系统，主要功能包括：

- ✅ 树莓派5 + IMX477摄像头模块支持
- ✅ 外接显示屏实时预览
- ✅ 键盘鼠标参数控制
- ✅ 4056x3040最高分辨率RAW视频录制
- ✅ 24fps录制帧率
- ✅ 12位位深度
- ✅ 录制文件自动管理
- ✅ 系统性能优化

## 硬件配置

| 组件 | 型号/规格 | 说明 |
|------|-----------|------|
| 树莓派 | 树莓派5 8GB版本 | 核心计算平台 |
| 摄像头 | 官方IMX477模块 | 1230万像素，RAW输出 |
| 显示屏 | 外接笔记本显示屏 | 分辨率建议1920x1080及以上 |
| 输入设备 | USB键盘和鼠标 | 用于参数控制 |
| 存储 | 高速MicroSD卡（推荐128GB+） | 用于系统和录制文件存储 |
| 电源 | 树莓派5官方电源或5V3A电源 | 确保稳定供电 |
| 连接 | HDMI线、摄像头排线 | 连接显示屏和摄像头 |

## 软件安装

### 1. 系统安装

1. 下载最新的Raspberry Pi OS (64-bit) Lite
2. 使用Raspberry Pi Imager将系统烧录到MicroSD卡
3. 配置Wi-Fi和启用SSH（可选）
4. 启动树莓派并完成初始设置

### 2. 基础依赖安装

```bash
# 更新系统
 sudo apt-get update
 sudo apt-get upgrade -y

# 安装开发工具
 sudo apt-get install -y build-essential cmake git

# 安装图形和视频库
 sudo apt-get install -y libsdl2-dev libsdl2-ttf-dev libgles2-mesa-dev

# 安装摄像头相关依赖
 sudo apt-get install -y libcamera-dev libcamera-apps

# 安装其他必要依赖
 sudo apt-get install -y libzram-tools
```

### 3. CinePI SDK安装

```bash
# 克隆CinePI SDK仓库
 git clone https://github.com/cinepi/cinepi-sdk.git

# 进入仓库目录
 cd cinepi-sdk

# 创建构建目录
 mkdir build
 cd build

# 配置和编译
 cmake ..
 make -j4

# 安装SDK
 sudo make install
```

### 4. 项目代码下载

```bash
# 克隆项目代码
 git clone https://github.com/your-username/cinepi-raspberry-pi5.git
 cd cinepi-raspberry-pi5

# 给脚本添加执行权限
 chmod +x *.sh
```

## 功能实现

### 1. 摄像头预览功能

**编译所有应用：**

使用统一构建脚本编译所有应用和共享模块：

```bash
./build.sh
```

**（可选）分别编译应用：**

```bash
# 只编译预览应用
./build_preview.sh

# 只编译录制应用
./build_recorder.sh
```

**运行预览应用：**

```bash
./cinepi_preview
# 或
./build/cinepi_preview
```

**预览控制按键：**
- `空格键`：开始/停止预览
- `方向键上/下`：调整曝光补偿
- `方向键左/右`：调整ISO
- `W键`：循环切换白平衡
- `ESC键`：退出应用

### 2. RAW视频录制功能

**运行录制应用：**

```bash
./cinepi_raw_recorder [录制目录]
# 或
./build/cinepi_raw_recorder [录制目录]
# 默认录制目录：/home/pi/cinepi_recordings
```

**录制控制按键：**
- `空格键`：开始/停止录制
- `方向键上/下`：调整曝光补偿
- `方向键左/右`：调整ISO
- `W键`：循环切换白平衡
- `ESC键`：退出应用

### 3. 存储配置和文件管理

**设置录制目录：**

```bash
./storage_manager.sh --setup
```

**检查存储空间：**

```bash
./storage_manager.sh --check
```

**列出录制文件：**

```bash
./storage_manager.sh --list
```

**清理旧文件：**

```bash
./storage_manager.sh --clean
```

**自定义录制目录：**

```bash
./storage_manager.sh --directory /mnt/usb --check
```

### 4. 系统性能测试和优化

**运行完整性能测试：**

```bash
./performance_tester.sh --benchmark
```

**测试系统资源使用：**

```bash
./performance_tester.sh --system
```

**测试摄像头性能：**

```bash
./performance_tester.sh --camera
```

**测试录制性能：**

```bash
./performance_tester.sh --record
```

**应用系统优化：**

```bash
sudo ./performance_tester.sh --optimize
```

## 系统架构

```
┌───────────────────────┐     ┌───────────────────────┐     ┌───────────────────────┐
│  树莓派5 8GB          │     │  IMX477摄像头模块      │     │  外接显示屏           │
│                       │     │                       │     │                       │
│  ┌───────────────┐    │     │                       │     │                       │
│  │ CinePI SDK    │    │────▶│                       │     │                       │
│  └───────────────┘    │     │                       │     │                       │
│                       │     └───────────────────────┘     │                       │
│  ┌───────────────┐    │                                   │                       │
│  │ 预览应用      │────┼──────────────────────────────────▶│                       │
│  └───────────────┘    │                                   │                       │
│                       │                                   │                       │
│  ┌───────────────┐    │     ┌───────────────────────┐     │                       │
│  │ 录制应用      │────┼────▶│  存储系统             │     │                       │
│  └───────────────┘    │     │                       │     │                       │
│                       │     │ ┌───────────────────┐ │     │                       │
│  ┌───────────────┐    │     │ │ 录制文件管理       │ │     │                       │
│  │ 参数控制系统   │────┼────▶│ └───────────────────┘ │     │                       │
│  └───────────────┘    │     └───────────────────────┘     │                       │
│                       │                                   │                       │
│  ┌───────────────┐    │                                   │                       │
│  │ 性能优化系统   │    │                                   │                       │
│  └───────────────┘    │                                   │                       │
└───────────────────────┘                                   └───────────────────────┘
```

## 技术参数

### 摄像头参数
- **传感器型号**：IMX477
- **像素**：1230万（4056x3040）
- **传感器尺寸**：1/2.3英寸
- **像素大小**：1.55μm × 1.55μm
- **RAW输出格式**：12位RAW
- **帧速率**：24fps（4056x3040），最高60fps（降低分辨率）

### 录制参数
- **最高分辨率**：4056x3040
- **录制帧率**：24fps
- **位深度**：12位
- **文件格式**：RAW
- **默认录制目录**：/home/pi/cinepi_recordings

## 常见问题

### 1. 摄像头无法识别

**解决方法：**
- 检查摄像头排线是否正确连接
- 确保已启用摄像头接口：`sudo raspi-config` → Interface Options → Camera → Enable
- 更新系统：`sudo apt-get update && sudo apt-get upgrade -y`
- 检查摄像头是否被占用：`sudo lsof /dev/video0`

### 2. 预览窗口黑屏

**解决方法：**
- 检查HDMI连接是否正常
- 确保显示屏分辨率设置正确
- 检查GPU内存配置：`cat /boot/firmware/config.txt`，确保`gpu_mem=256`或更高
- 重新启动应用程序

### 3. 录制文件大小异常

**解决方法：**
- 检查存储设备是否有足够空间：`df -h`
- 检查存储设备读写速度：`hdparm -tT /dev/mmcblk0`
- 确保使用高速MicroSD卡（UHS-II级别推荐）

### 4. 系统性能问题

**解决方法：**
- 应用系统优化：`sudo ./performance_tester.sh --optimize`
- 启用CPU性能模式：`echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor`
- 增加GPU内存到256MB或更高
- 关闭不需要的系统服务

## 项目文件说明

| 文件名 | 说明 |
|--------|------|
| `cinepi_preview.cpp` | 摄像头预览应用源代码 |
| `cinepi_raw_recorder.cpp` | RAW视频录制应用源代码 |
| `build.sh` | 统一编译脚本（编译所有应用和共享模块） |
| `build_preview.sh` | 预览应用编译脚本（兼容旧版本） |
| `build_recorder.sh` | 录制应用编译脚本（兼容旧版本） |
| `storage_manager.sh` | 存储配置和文件管理脚本 |
| `performance_tester.sh` | 系统性能测试和优化脚本 |
| `src/shared/sdl_helper.h` | SDL2辅助类头文件，提供窗口、渲染器和纹理管理 |
| `src/shared/sdl_helper.cpp` | SDL2辅助类实现文件 |
| `src/shared/camera_controller.h` | 摄像头控制器类头文件，提供摄像头初始化和参数设置 |
| `src/shared/camera_controller.cpp` | 摄像头控制器类实现文件 |
| `cinepi_raspberry_pi5_solution.md` | 详细解决方案文档 |
| `system_setup_guide.md` | 系统安装和基础配置指南 |
| `README.md` | 项目说明文档 |

## GitHub仓库信息

项目已经成功克隆了以下GitHub仓库：

1. **CinePi仓库**
   - URL: https://github.com/CinePi/CinePi.git
   - 位置: `camera001/CinePi/`
   - 用途: CinePI项目的主仓库，包含项目信息和资源
   - 关键文件: 
     - `CinePi/README.md`: 项目说明文档

2. **libcamera仓库**
   - URL: https://github.com/raspberrypi/libcamera.git
   - 位置: `camera001/libcamera/`
   - 用途: 树莓派官方摄像头驱动库，提供对IMX477等摄像头的支持

3. **libcamera-apps仓库**
   - URL: https://github.com/raspberrypi/libcamera-apps.git
   - 位置: `camera001/libcamera-apps/`
   - 用途: 基于libcamera的应用程序集合，包含RAW视频录制功能
   - 关键文件: 
     - `libcamera-apps/apps/rpicam_raw.cpp`: 原始RAW视频录制应用程序
     - `libcamera-apps/apps/cinepi_raw_app.cpp`: 我们的CinePI RAW视频录制应用程序
     - `libcamera-apps/core/rpicam_app.hpp`: 应用程序核心框架
     - `libcamera-apps/core/rpicam_encoder.hpp`: 编码器核心框架

## CinePI RAW视频录制应用

### 功能特点
- 基于libcamera-apps框架开发的高质量RAW视频录制应用
- 实时SDL2预览窗口，支持高分辨率显示
- 键盘控制参数调整（曝光、ISO、白平衡）
- 录制状态实时显示
- 支持IMX477摄像头的4056x3040高分辨率RAW录制

### 安装方法
1. 确保系统已经安装了所有必要的依赖
2. 运行统一构建脚本：`./build.sh`
3. 等待构建完成

### 使用方法
1. 启动应用程序：`cinepi-raw-app`
2. 使用键盘控制参数：
   - **Q**: 退出应用程序
   - **R**: 开始/停止录制
   - **↑/↓**: 调整曝光补偿
   - **←/→**: 调整ISO值
   - **W/S**: 调整白平衡

### 构建说明
我们已经在libcamera-apps仓库中添加了自定义的应用程序`cinepi_raw_app.cpp`，并更新了构建配置以支持SDL2依赖。构建脚本会自动处理所有依赖和编译过程。

## 后续开发建议

1. **性能优化**
   - 调整libcamera的缓存和缓冲区设置
   - 优化SDL2预览窗口的渲染性能
   - 考虑使用硬件加速的视频编解码器

2. **功能扩展**
   - 添加视频格式转换功能
   - 实现定时录制功能
   - 添加更丰富的参数控制选项

3. **用户体验改进**
   - 优化UI界面，添加更多实时信息显示
   - 实现配置文件保存和加载功能
   - 添加错误处理和日志记录

4. **系统集成**
   - 实现开机自动启动功能
   - 添加远程控制接口
   - 集成到树莓派的系统菜单中


## 下一步计划

- [ ] 支持更高帧率录制（48fps/60fps）
- [ ] 添加RAW文件转换为DNG格式功能
- [ ] 支持外部存储设备自动挂载
- [ ] 实现远程控制功能
- [ ] 添加实时视频滤镜效果
- [ ] 开发图形化配置界面

## 参考资料

- [CinePI GitHub仓库](https://github.com/cinepi/cinepi-sdk)
- [树莓派5官方文档](https://www.raspberrypi.com/documentation/computers/raspberry-pi-5.html)
- [IMX477摄像头模块规格](https://www.arducam.com/product/12-3mp-imx477-camera-module/)
- [Raspberry Pi Camera API](https://www.raspberrypi.com/documentation/accessories/camera.html)

## 许可证

本项目基于MIT许可证开源。

## 贡献

欢迎提交Issue和Pull Request！

---

**作者：** CinePI开发团队  
**日期：** 2025年1月
**版本：** v1.0.0