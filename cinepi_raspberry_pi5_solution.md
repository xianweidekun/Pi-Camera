# CinePI基于树莓派5的RAW视频录制系统方案

## 一、硬件配置方案

### 1. 树莓派5 8G版本
- **处理器**：2.4GHz四核64位Arm Cortex-A76 CPU
- **内存**：8GB LPDDR4X-4267 SDRAM
- **存储**：推荐使用128GB或更大容量的Class 10/UHS-I microSD卡
- **视频输出**：2个HDMI 2.0端口，支持4K60p
- **USB接口**：3个USB 3.0端口（用于高速存储和外设），2个USB 2.0端口
- **网络**：千兆以太网，双频Wi-Fi 5
- **摄像头接口**：2个MIPI CSI-2接口（支持IMX477摄像头）

### 2. IMX477摄像头模块
- **传感器**：索尼IMX477R
- **像素**：1230万（4056×3040）
- **像素尺寸**：1.55μm×1.55μm
- **输出格式**：RAW12/10/8，COMP8
- **接口**：MIPI CSI-2（与树莓派5兼容）
- **镜头选项**：支持C/CS型镜头或M12镜头
- **特点**：高灵敏度，支持长时间曝光，适合RAW视频录制

### 3. 外接显示屏
- **类型**：HDMI显示器或触控屏
- **分辨率**：建议1920×1080p或更高（支持4K）
- **尺寸**：10-15英寸便携屏（适合移动拍摄）或24英寸以上桌面显示器
- **接口**：HDMI 2.0
- **刷新率**：至少60Hz

### 4. 键盘鼠标
- **类型**：USB有线或无线键鼠套装
- **连接**：通过树莓派5的USB 3.0端口连接
- **特点**：无线套装更适合移动使用，有线套装更稳定

## 二、系统安装和基础配置

### 1. 操作系统安装
- 使用Raspberry Pi Imager安装最新的Raspberry Pi OS (64-bit)
- 选择"Raspberry Pi OS (64-bit)"或"Raspberry Pi OS Lite (64-bit)"（后者更轻量）
- 配置Wi-Fi、SSH等基本设置

### 2. 基础依赖安装
```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y git cmake build-essential python3-pip
sudo apt install -y libssl-dev libcurl4-openssl-dev libjpeg-dev libpng-dev
sudo apt install -y libv4l-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
```

## 三、摄像头驱动和CinePI SDK安装

### 1. 启用摄像头接口
```bash
sudo raspi-config
```
在配置菜单中选择：Interfacing Options → Camera → Yes，然后重启树莓派

### 2. 安装CinePI SDK
```bash
git clone https://github.com/cinepi/cinepi-sdk.git
cd cinepi-sdk
mkdir build && cd build
cmake ..
make -j4
sudo make install
```

### 3. 安装IMX477驱动（如需要）
```bash
sudo apt install -y rpi-update
sudo rpi-update
sudo reboot
```

## 四、摄像头预览功能实现

### 1. 使用GStreamer实现预览
```bash
gst-launch-1.0 libcamerasrc ! videoconvert ! autovideosink
```

### 2. 使用CinePI SDK实现高级预览
```c
#include <cinepi/cinepi.h>

int main() {
    CinePI camera;
    if (!camera.initialize()) {
        return -1;
    }
    
    // 设置预览分辨率
    camera.setPreviewResolution(1920, 1080);
    
    // 启动预览
    if (!camera.startPreview()) {
        return -1;
    }
    
    // 等待用户输入
    printf("按Enter键停止预览...");
    getchar();
    
    // 停止预览
    camera.stopPreview();
    camera.shutdown();
    
    return 0;
}
```

## 五、键盘鼠标控制参数实现

### 1. 使用SDL2实现键鼠输入
```bash
sudo apt install -y libsdl2-dev libsdl2-ttf-dev
```

### 2. 示例代码
```c
#include <SDL2/SDL.h>
#include <cinepi/cinepi.h>

int main() {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return -1;
    }
    
    // 创建窗口
    SDL_Window *window = SDL_CreateWindow("CinePI Control", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Quit();
        return -1;
    }
    
    // 初始化CinePI
    CinePI camera;
    if (!camera.initialize()) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // 主循环
    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_KEYDOWN) {
                // 处理键盘输入
                switch (e.key.keysym.sym) {
                    case SDLK_q:
                        running = false;
                        break;
                    case SDLK_UP:
                        // 增加曝光补偿
                        camera.adjustExposure(0.5);
                        break;
                    case SDLK_DOWN:
                        // 减少曝光补偿
                        camera.adjustExposure(-0.5);
                        break;
                    case SDLK_r:
                        // 开始/停止录制
                        if (camera.isRecording()) {
                            camera.stopRecording();
                        } else {
                            camera.startRecording("/path/to/output.raw");
                        }
                        break;
                }
            }
        }
    }
    
    // 清理
    camera.shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
```

## 六、RAW视频录制功能实现

### 1. 基本录制命令
```bash
# 使用CinePI命令行工具录制RAW视频
cinepi-record --output /path/to/output.raw --resolution 4056x3040 --format raw12 --fps 24 --duration 60
```

### 2. 使用SDK录制
```c
#include <cinepi/cinepi.h>

int main() {
    CinePI camera;
    if (!camera.initialize()) {
        return -1;
    }
    
    // 设置录制参数
    camera.setRecordingResolution(4056, 3040);
    camera.setRecordingFormat(RAW12);
    camera.setFPS(24);
    
    // 设置存储路径
    std::string outputPath = "/path/to/output.raw";
    
    // 开始录制
    if (!camera.startRecording(outputPath)) {
        camera.shutdown();
        return -1;
    }
    
    // 录制60秒
    printf("正在录制60秒...\n");
    sleep(60);
    
    // 停止录制
    camera.stopRecording();
    
    printf("录制完成：%s\n", outputPath.c_str());
    
    camera.shutdown();
    
    return 0;
}
```

## 七、存储配置和文件管理

### 1. 存储配置
- **系统盘**：microSD卡，安装Raspberry Pi OS
- **视频存储**：建议使用外接USB 3.0 SSD或高速U盘
  ```bash
  # 格式化外接存储为ext4格式
  sudo mkfs.ext4 /dev/sda1
  
  # 创建挂载点
  sudo mkdir /mnt/cinepi-storage
  
  # 挂载存储
  sudo mount /dev/sda1 /mnt/cinepi-storage
  
  # 设置自动挂载
  sudo blkid  # 获取UUID
  sudo nano /etc/fstab
  # 添加以下行（替换UUID）
  UUID=your-uuid /mnt/cinepi-storage ext4 defaults,noatime 0 2
  ```

### 2. 文件管理
- **录制目录**：/mnt/cinepi-storage/recordings
- **预览目录**：/mnt/cinepi-storage/previews
- **日志目录**：/mnt/cinepi-storage/logs

```bash
# 创建目录结构
sudo mkdir -p /mnt/cinepi-storage/{recordings,previews,logs}

# 设置权限
sudo chown -R pi:pi /mnt/cinepi-storage
```

## 八、测试和优化系统性能

### 1. 性能测试
```bash
# 测试SD卡速度
sudo hdparm -t /dev/mmcblk0

# 测试外接存储速度
sudo hdparm -t /dev/sda1

# 监控系统资源
top
htop
```

### 2. 优化建议
- 启用ZRAM以提高内存性能
  ```bash
  sudo apt install -y zram-tools
  sudo nano /etc/default/zramswap
  # 设置PERCENTAGE=50
  sudo systemctl restart zramswap
  ```

- 调整GPU内存分配
  ```bash
  sudo nano /boot/config.txt
  # 添加或修改
  gpu_mem=256
  ```

- 禁用不必要的服务
  ```bash
  sudo systemctl disable bluetooth
  sudo systemctl disable wpa_supplicant  # 如果不使用Wi-Fi
  ```

## 九、完整系统架构

```
┌─────────────────────────────────────────────────────────┐
│                     树莓派5 8G                          │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────────┐ │
│  │ IMX477  │  │ HDMI    │  │ USB     │  │ 系统存储     │ │
│  │ 摄像头  │  │ 显示屏  │  │ 键鼠/SSD │  │ (microSD)   │ │
│  └─────────┘  └─────────┘  └─────────┘  └─────────────┘ │
│                                                         │
│  ┌─────────────────────────────────────────────────────┐ │
│  │                    软件层                            │ │
│  │  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐ │ │
│  │  │ CinePI  │  │ 预览    │  │ 控制    │  │ 文件管理 │ │ │
│  │  │ SDK     │  │ 模块    │  │ 模块    │  │ 模块     │ │ │
│  │  └─────────┘  └─────────┘  └─────────┘  └─────────┘ │ │
│  └─────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

## 十、使用说明

### 1. 开机启动
- 连接所有硬件设备
- 插入电源，树莓派5自动启动

### 2. 启动CinePI应用
```bash
# 运行CinePI主应用
cinepi-app
```

### 3. 控制界面说明
- **预览窗口**：显示摄像头实时画面
- **控制按钮**：
  - 开始/停止录制
  - 调整曝光、ISO、白平衡等参数
  - 切换分辨率和帧率
  - 查看录制历史

### 4. 快捷键
- `R`：开始/停止录制
- `↑/↓`：调整曝光补偿
- `←/→`：调整ISO
- `W`：切换白平衡模式
- `Q`：退出应用

## 十一、故障排除

### 1. 摄像头不工作
- 检查摄像头连接是否正确
- 确保已启用摄像头接口
- 检查摄像头驱动是否安装正确

### 2. 预览卡顿
- 降低预览分辨率
- 关闭不必要的应用程序
- 确保使用高速存储设备

### 3. 录制失败
- 检查存储设备是否有足够空间
- 检查存储设备权限
- 降低录制分辨率或帧率

---

本方案详细说明了基于树莓派5和CinePI的RAW视频录制系统的硬件配置、软件实现和使用方法。通过该方案，用户可以实现高质量的RAW视频录制，并通过外接显示屏和键鼠进行实时控制。