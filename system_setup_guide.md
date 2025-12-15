# 系统安装和基础配置指南

## 一、Raspberry Pi OS安装

### 1. 准备工作
- 树莓派5 8G版本
- 256GB M2 NVMe SSD硬盘
- M2 NVMe SSD扩展板（兼容树莓派5）
- 128GB或更大容量的Class 10/UHS-I microSD卡（用于初始安装）
- 一台Windows/macOS/Linux电脑
- SD卡读卡器

### 2. 下载Raspberry Pi Imager
访问[Raspberry Pi官方网站](https://www.raspberrypi.com/software/)下载并安装Raspberry Pi Imager。

### 3. 安装Raspberry Pi OS到SD卡（初始安装）
1. 打开Raspberry Pi Imager
2. 选择"CHOOSE OS" → "Raspberry Pi OS (64-bit)"
3. 选择"CHOOSE STORAGE" → 选择你的microSD卡
4. 点击"WRITE"按钮开始写入系统镜像
5. 等待写入完成，然后安全弹出SD卡

### 4. 配置M2 SSD支持
在将SD卡插入树莓派之前，配置M2 SSD支持：

1. 将SD卡重新插入电脑
2. 在boot分区中创建或编辑`config.txt`文件，添加以下内容：
   ```txt
   # 启用M2 NVMe SSD支持
   dtparam=nvme
   ```

### 4. 初始系统配置（可选）
在将SD卡插入树莓派之前，你可以预先配置一些设置：

1. 将SD卡重新插入电脑
2. 在boot分区中创建以下文件：
   - **ssh**：空文件，用于启用SSH访问
   - **wpa_supplicant.conf**：用于配置Wi-Fi连接

```conf
# wpa_supplicant.conf内容
country=CN
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1

network={
    ssid="你的Wi-Fi名称"
    psk="你的Wi-Fi密码"
    key_mgmt=WPA-PSK
}
```

## 二、树莓派初始设置

### 1. 启动树莓派
1. 将microSD卡插入树莓派5
2. 连接HDMI显示器、键盘和鼠标
3. 连接电源，树莓派自动启动

### 2. 完成系统设置向导
按照屏幕上的提示完成以下设置：
- 设置国家、语言和时区
- 设置键盘布局
- 更改默认密码（pi用户）
- 连接Wi-Fi网络
- 安装系统更新

## 三、系统安装到M2 SSD

### 1. 安装基础依赖
```bash
sudo apt update && sudo apt upgrade -y
```

### 2. 安装必要的开发工具
```bash
sudo apt install -y git cmake build-essential python3-pip
```

### 3. 安装图形和视频相关依赖
```bash
sudo apt install -y libssl-dev libcurl4-openssl-dev libjpeg-dev libpng-dev
sudo apt install -y libv4l-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
sudo apt install -y libgstreamer-plugins-good1.0-dev libgstreamer-plugins-bad1.0-dev
```

### 4. 安装SDL2（用于图形界面和输入处理）
```bash
sudo apt install -y libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
```

### 5. 安装其他必要工具
```bash
sudo apt install -y htop iotop nmon  # 系统监控工具
sudo apt install -y udisks2  # 自动挂载存储设备
sudo apt install -y zram-tools  # 内存优化
```

### 6. 安装系统到M2 SSD
1. 查看存储设备：
   ```bash
   lsblk
   ```
   找到M2 SSD设备（通常为`nvme0n1`）

2. 使用Raspberry Pi Imager将系统克隆到SSD：
   - 选择SD卡作为源设备
   - 选择SSD作为目标设备
   - 点击"Write"开始克隆

3. 或者使用`dd`命令克隆系统：
   ```bash
   sudo dd if=/dev/mmcblk0 of=/dev/nvme0n1 bs=4M status=progress
   ```

### 7. 配置SSD启动
1. 编辑`/boot/config.txt`文件：
   ```bash
   sudo nano /boot/config.txt
   ```

2. 添加以下内容设置启动顺序：
   ```txt
   # 设置启动顺序，优先从NVMe启动
   boot_order=0xf41
   ```

3. 重启树莓派：
   ```bash
   sudo reboot
   ```

### 8. 验证SSD启动
重启后，运行以下命令验证系统是否从SSD启动：
```bash
df -h /
```
如果显示`/dev/nvme0n1p2`，说明系统已从SSD启动

## 四、系统配置优化

### 1. 扩展SSD文件系统
1. 运行以下命令扩展文件系统到整个SSD：
   ```bash
   sudo raspi-config
   ```

2. 选择"Advanced Options" > "Expand Filesystem"

3. 重启系统：
   ```bash
   sudo reboot
   ```

### 2. 启用摄像头接口
```bash
sudo raspi-config
```
在配置菜单中选择：
- `Interfacing Options` → `Camera` → `Yes`
然后重启树莓派：
```bash
sudo reboot
```

### 3. 调整GPU内存分配
```bash
sudo nano /boot/config.txt
```
添加或修改以下行：
```txt
gpu_mem=256
```
保存并退出，然后重启树莓派。

### 4. 启用ZRAM
```bash
sudo nano /etc/default/zramswap
```
设置：
```txt
PERCENTAGE=50
```
然后重启ZRAM服务：
```bash
sudo systemctl restart zramswap
```

### 5. 禁用不必要的服务
```bash
sudo systemctl disable bluetooth  # 如果不使用蓝牙
sudo systemctl disable wpa_supplicant  # 如果不使用Wi-Fi
sudo systemctl disable avahi-daemon  # 如果不需要零配置网络
```

### 6. 优化文件系统
```bash
sudo tune2fs -o discard /dev/nvme0n1p2  # 启用TRIM（SSD支持）
sudo nano /etc/fstab
```
在根分区的挂载选项中添加`noatime`：
```txt
PARTUUID=xxxx-xxxx  /               ext4    defaults,noatime  0       1
```

## 五、验证系统配置

### 1. 检查系统版本
```bash
cat /etc/os-release
```

### 2. 检查内存和CPU信息
```bash
free -h
lscpu
```

### 3. 检查摄像头是否被识别
```bash
vcgencmd get_camera
```
如果输出显示`detected=1`，说明摄像头已被识别。

### 4. 测试摄像头
```bash
libcamera-hello -t 5000
```
这将显示摄像头预览5秒钟。

## 六、项目部署

完成系统安装和基础配置后，按照以下步骤部署CinePI项目：

### 1. 克隆项目仓库
```bash
git clone https://github.com/xianweidekun/camera001.git
cd camera001
```

### 2. 构建应用程序
运行自动化构建脚本：
```bash
chmod +x build_cinepi_app.sh
./build_cinepi_app.sh
```

### 3. 测试应用程序
启动CinePI RAW视频录制应用：
```bash
cinepi-raw-app
```

### 4. 键盘控制说明
- **Q**: 退出应用程序
- **R**: 开始/停止录制
- **↑/↓**: 调整曝光补偿
- **←/→**: 调整ISO值
- **W/S**: 调整白平衡

## 七、存储管理

### 1. 创建视频存储目录
```bash
sudo mkdir -p /media/cinepi/videos
sudo chown pi:pi /media/cinepi/videos
```

### 2. 设置自动挂载（可选）
如果使用额外的存储设备：
1. 查看设备UUID：
   ```bash
   sudo blkid
   ```

2. 编辑`/etc/fstab`文件：
   ```bash
   sudo nano /etc/fstab
   ```

3. 添加挂载配置：
   ```txt
   UUID=<your-uuid> /media/cinepi/videos ext4 defaults,noatime,discard 0 2
   ```

4. 重新挂载：
   ```bash
   sudo mount -a
   ```

## 八、性能监控和维护

### 1. 系统监控
使用以下工具监控系统性能：
```bash
# 查看CPU温度
vcgencmd measure_temp

# 查看系统负载
top

# 查看内存使用情况
htop

# 查看磁盘I/O情况
iotop
```

### 2. 定期更新
```bash
sudo apt update
sudo apt upgrade -y
sudo apt dist-upgrade -y
```

### 3. 备份系统
```bash
# 备份系统到外部存储
sudo dd if=/dev/nvme0n1 of=/media/usb/backup.img bs=4M status=progress
```

请参考主方案文档中的相应部分获取详细指导。