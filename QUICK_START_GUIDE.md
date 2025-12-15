# CinePI项目快速启动指南

## 树莓派5构建项目的文档阅读顺序

### 1. **第一步：系统安装与基础配置**
**文档：`system_setup_guide.md`**

这是构建项目的**起始文档**，包含以下关键步骤：
- Raspberry Pi OS安装（SD卡初始安装）
- M2 SSD支持配置与系统迁移
- 系统基础设置（语言、时区、密码等）
- 必要开发工具安装（git、cmake、build-essential等）
- 摄像头接口启用与GPU内存调整
- 系统性能优化

**重要性**：确保树莓派5硬件环境和系统配置正确，为后续项目构建奠定基础。

### 2. **第二步：项目依赖安装**
**文档：`dependencies_install_guide.md`**

在完成系统配置后，安装项目所需的所有依赖：
- libcamera开发库
- SDL2图形库
- 视频编码解码库
- 其他必要的系统依赖

### 3. **第三步：项目构建**
**文档：`build_troubleshooting.md`**

学习如何正确构建项目并解决常见问题：
- 正确使用meson和ninja构建工具
- 解决依赖缺失问题（如libexif、libavcodec等）
- 处理构建错误和警告

### 4. **第四步：运行与测试**
**文档：`running_guide.md`**

项目构建完成后，了解如何运行和使用：
- CinePI RAW应用程序启动命令
- 键盘控制说明（录制、曝光调整等）
- 常见运行错误排查
- 性能优化建议

## 一键构建脚本

如果你已经完成了系统配置，可以直接使用我们提供的自动化脚本快速构建项目：

```bash
# 进入项目目录
cd ~/camera001

# 运行依赖安装和构建脚本
chmod +x install_deps_and_build.sh
./install_deps_and_build.sh
```

## 常见问题快速解决

1. **meson命令未找到**：
   ```bash
   sudo apt install -y meson ninja-build
   ```

2. **libexif依赖缺失**：
   ```bash
   sudo apt install -y libexif-dev
   ```

3. **cinepi-raw-app命令未找到**：
   ```bash
   sudo ldconfig  # 更新动态链接库缓存
   export PATH=$PATH:/usr/local/bin  # 确保安装路径在PATH中
   ```

## 技术支持

如果在构建或运行过程中遇到问题，请参考以下文档：
- `build_troubleshooting.md`：构建问题排查
- `running_guide.md`：运行问题排查

也可以查看项目中的其他文档获取更详细的信息。

---

**开始你的CinePI项目之旅吧！** 🚀