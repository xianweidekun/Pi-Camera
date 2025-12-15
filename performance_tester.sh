#!/bin/bash

# CinePI 系统性能测试和优化脚本

# 默认配置
TEST_DURATION=10  # 测试持续时间 (秒)
RECORD_TEST_DURATION=5  # 录制测试持续时间 (秒)
RECORD_DIR="/home/pi/cinepi_recordings"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 显示帮助信息
show_help() {
    echo -e "${BLUE}CinePI 系统性能测试和优化脚本${NC}"
    echo -e "用于测试和优化 CinePI RAW 视频录制系统的性能"
    echo -e ""
    echo -e "选项:"
    echo -e "  -h, --help              显示此帮助信息"
    echo -e "  -b, --benchmark         运行完整性能测试"
    echo -e "  -s, --system            测试系统资源使用情况"
    echo -e "  -c, --camera            测试摄像头性能"
    echo -e "  -r, --record            测试视频录制性能"
    echo -e "  -o, --optimize          应用系统优化"
    echo -e "  -d, --directory DIR     指定录制目录 (默认: $RECORD_DIR)"
    echo -e "  -t, --time SECONDS      测试持续时间 (默认: $TEST_DURATION 秒)"
    echo -e ""
    echo -e "示例:"
    echo -e "  $0 --benchmark"                # 运行完整性能测试
    echo -e "  $0 --system"                   # 测试系统资源使用情况
    echo -e "  $0 --record --time 10"         # 测试10秒录制性能
    echo -e "  $0 --optimize"                 # 应用系统优化
}

# 测试系统资源使用情况
test_system_resources() {
    local duration=${1:-$TEST_DURATION}
    
    echo -e "${BLUE}=== 系统资源使用情况测试 ===${NC}"
    echo -e "测试持续时间: $duration 秒"
    echo -e ""
    
    # 显示系统基本信息
    echo -e "${PURPLE}系统信息:${NC}"
    uname -a
    echo -e ""
    
    # 显示CPU信息
    echo -e "${PURPLE}CPU信息:${NC}"
    lscpu | grep -E "Model name|CPU(s)|Thread(s) per core"
    echo -e ""
    
    # 显示内存信息
    echo -e "${PURPLE}内存信息:${NC}"
    free -h
    echo -e ""
    
    # 显示磁盘信息
    echo -e "${PURPLE}磁盘信息:${NC}"
    df -h
    echo -e ""
    
    # 测试CPU和内存使用情况
    echo -e "${PURPLE}实时资源监控:${NC}"
    echo -e "正在监控 $duration 秒..."
    
    # 使用top命令监控资源使用
    top -b -d 1 -n $duration | tail -n +8 > /tmp/resource_monitor.txt
    
    # 计算平均值
    local cpu_sum=0
    local mem_sum=0
    local count=0
    
    while read -r line; do
        if [[ $line =~ ^%Cpu ]]; then
            # 提取CPU使用率
            local cpu=$(echo $line | awk '{print $2}')
            cpu_sum=$(echo "$cpu_sum + $cpu" | bc)
            count=$((count + 1))
        fi
    done < /tmp/resource_monitor.txt
    
    if [ $count -gt 0 ]; then
        local cpu_avg=$(echo "scale=2; $cpu_sum / $count" | bc)
        echo -e "平均CPU使用率: ${CYAN}$cpu_avg%${NC}"
    else
        echo -e "平均CPU使用率: ${RED}无法获取${NC}"
    fi
    
    # 清理临时文件
    rm -f /tmp/resource_monitor.txt
    
    echo -e ""
    echo -e "${GREEN}系统资源测试完成!${NC}"
}

# 测试摄像头性能
test_camera_performance() {
    local duration=${1:-$TEST_DURATION}
    
    echo -e "${BLUE}=== 摄像头性能测试 ===${NC}"
    echo -e "测试持续时间: $duration 秒"
    echo -e ""
    
    # 检查摄像头是否可用
    if ! libcamera-hello --list-cameras > /dev/null 2>&1; then
        echo -e "${RED}✗ 摄像头不可用或未正确配置${NC}"
        echo -e "请检查摄像头连接和驱动程序"
        return 1
    fi
    
    # 显示摄像头信息
    echo -e "${PURPLE}摄像头信息:${NC}"
    libcamera-hello --list-cameras
    echo -e ""
    
    # 测试摄像头帧率
    echo -e "${PURPLE}摄像头帧率测试:${NC}"
    echo -e "正在测试 $duration 秒..."
    
    # 使用libcamera-vid测试帧率
    libcamera-vid -t $((duration * 1000)) --framerate 30 --width 1920 --height 1080 --nopreview --metadata - | grep -A 5 "Frame rate"
    
    echo -e ""
    echo -e "${GREEN}摄像头性能测试完成!${NC}"
}

# 测试视频录制性能
test_record_performance() {
    local duration=${1:-$RECORD_TEST_DURATION}
    local record_dir=${2:-$RECORD_DIR}
    
    echo -e "${BLUE}=== 视频录制性能测试 ===${NC}"
    echo -e "测试持续时间: $duration 秒"
    echo -e "录制目录: $record_dir"
    echo -e ""
    
    # 检查录制目录是否存在
    if [ ! -d "$record_dir" ]; then
        echo -e "${YELLOW}! 录制目录不存在，正在创建...${NC}"
        mkdir -p "$record_dir"
    fi
    
    # 生成测试文件名
    local test_filename="test_recording_$(date +%Y%m%d_%H%M%S).raw"
    local test_filepath="$record_dir/$test_filename"
    
    # 记录开始时间和磁盘使用情况
    local start_time=$(date +%s)
    local start_disk_usage=$(df -k "$record_dir" | tail -1 | awk '{print $3}')
    
    echo -e "${PURPLE}开始录制测试:${NC} $test_filepath"
    
    # 使用cinepi_raw_recorder进行录制测试（如果已安装）
    if command -v cinepi_raw_recorder &> /dev/null; then
        # 运行录制应用
        timeout $duration ./cinepi_raw_recorder "$record_dir" > /dev/null 2>&1
        
        # 检查录制是否成功
        if [ -f "$test_filepath" ]; then
            local end_time=$(date +%s)
            local end_disk_usage=$(df -k "$record_dir" | tail -1 | awk '{print $3}')
            
            local actual_duration=$((end_time - start_time))
            local file_size=$(stat -c %s "$test_filepath")
            local disk_used=$((end_disk_usage - start_disk_usage))
            
            echo -e ""
            echo -e "${GREEN}✓ 录制测试成功!${NC}"
            echo -e "实际录制时间: ${CYAN}$actual_duration 秒${NC}"
            echo -e "文件大小: ${CYAN}$((file_size / 1024 / 1024)) MB${NC}"
            echo -e "磁盘使用: ${CYAN}$((disk_used / 1024)) MB${NC}"
            echo -e "录制速率: ${CYAN}$((file_size / actual_duration / 1024)) KB/s${NC}"
            
            # 清理测试文件
            rm -f "$test_filepath"
            echo -e "已清理测试文件"
        else
            echo -e "${RED}✗ 录制测试失败${NC}"
            echo -e "录制文件不存在: $test_filepath"
        fi
    else
        # 使用libcamera-vid作为替代测试
        echo -e "${YELLOW}! CinePI RAW录制应用未安装，使用libcamera-vid替代测试${NC}"
        
        local h264_test_file="$record_dir/test_recording_$(date +%Y%m%d_%H%M%S).h264"
        libcamera-vid -t $((duration * 1000)) --framerate 24 --width 4056 --height 3040 --codec h264 -o "$h264_test_file"
        
        if [ -f "$h264_test_file" ]; then
            local file_size=$(stat -c %s "$h264_test_file")
            echo -e ""
            echo -e "${GREEN}✓ 替代录制测试成功!${NC}"
            echo -e "文件大小: ${CYAN}$((file_size / 1024 / 1024)) MB${NC}"
            echo -e "录制速率: ${CYAN}$((file_size / duration / 1024)) KB/s${NC}"
            
            # 清理测试文件
            rm -f "$h264_test_file"
            echo -e "已清理测试文件"
        else
            echo -e "${RED}✗ 替代录制测试失败${NC}"
        fi
    fi
    
    echo -e ""
    echo -e "${GREEN}视频录制性能测试完成!${NC}"
}

# 应用系统优化
apply_optimizations() {
    echo -e "${BLUE}=== 应用系统优化 ===${NC}"
    echo -e ""
    
    # 检查是否为root用户
    if [ "$(id -u)" != "0" ]; then
        echo -e "${RED}✗ 需要root权限来应用系统优化${NC}"
        echo -e "请使用sudo运行此脚本: sudo $0 --optimize"
        return 1
    fi
    
    # 优化GPU内存
    echo -e "${PURPLE}1. 优化GPU内存配置${NC}"
    
    # 检查当前GPU内存配置
    local current_gpu_mem=$(grep "gpu_mem" /boot/firmware/config.txt 2>/dev/null | tail -1 | awk -F= '{print $2}')
    
    if [ -z "$current_gpu_mem" ] || [ "$current_gpu_mem" -lt "256" ]; then
        echo -e "${YELLOW}! 当前GPU内存配置不足，正在设置为256MB...${NC}"
        
        # 备份配置文件
        cp /boot/firmware/config.txt /boot/firmware/config.txt.bak.$(date +%Y%m%d_%H%M%S)
        
        # 添加或更新GPU内存配置
        if grep -q "gpu_mem=" /boot/firmware/config.txt; then
            sed -i "s/gpu_mem=.*/gpu_mem=256/" /boot/firmware/config.txt
        else
            echo "gpu_mem=256" >> /boot/firmware/config.txt
        fi
        
        echo -e "${GREEN}✓ GPU内存配置已更新为256MB${NC}"
    else
        echo -e "${GREEN}✓ GPU内存配置已经足够: $current_gpu_mem MB${NC}"
    fi
    
    echo -e ""
    
    # 优化CPU性能
    echo -e "${PURPLE}2. 优化CPU性能配置${NC}"
    
    # 检查是否已经启用性能模式
    if [ -f "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor" ]; then
        local current_governor=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor)
        
        if [ "$current_governor" != "performance" ]; then
            echo -e "${YELLOW}! 当前CPU频率调节策略: $current_governor${NC}"
            echo -e "${YELLOW}! 正在设置为performance模式...${NC}"
            
            # 设置所有CPU核心为performance模式
            for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
                echo "performance" > "$cpu"
            done
            
            echo -e "${GREEN}✓ CPU频率调节策略已设置为performance模式${NC}"
        else
            echo -e "${GREEN}✓ CPU频率调节策略已经是performance模式${NC}"
        fi
    else
        echo -e "${YELLOW}! 无法获取或设置CPU频率调节策略${NC}"
    fi
    
    echo -e ""
    
    # 优化磁盘I/O
    echo -e "${PURPLE}3. 优化磁盘I/O性能${NC}"
    
    # 检查当前调度器
    local disk=$(df -h | grep " / " | awk '{print $1}' | sed 's/[0-9]*$//')
    if [ -n "$disk" ] && [ -f "/sys/block/${disk##*/}/queue/scheduler" ]; then
        local current_scheduler=$(cat /sys/block/${disk##*/}/queue/scheduler | grep -o "\[[^]]*\]")
        
        if [[ $current_scheduler != *"mq-deadline"* ]]; then
            echo -e "${YELLOW}! 当前磁盘调度器: $current_scheduler${NC}"
            echo -e "${YELLOW}! 正在设置为mq-deadline...${NC}"
            
            echo "mq-deadline" > "/sys/block/${disk##*/}/queue/scheduler"
            echo -e "${GREEN}✓ 磁盘调度器已设置为mq-deadline${NC}"
        else
            echo -e "${GREEN}✓ 磁盘调度器已经是mq-deadline${NC}"
        fi
    else
        echo -e "${YELLOW}! 无法获取或设置磁盘调度器${NC}"
    fi
    
    echo -e ""
    
    # 优化ZRAM配置
    echo -e "${PURPLE}4. 优化ZRAM配置${NC}"
    
    # 检查ZRAM是否已启用
    if lsmod | grep -q zram; then
        echo -e "${GREEN}✓ ZRAM已经启用${NC}"
    else
        echo -e "${YELLOW}! ZRAM未启用，正在安装和配置...${NC}"
        
        # 安装zram-tools
        apt-get update
        apt-get install -y zram-tools
        
        # 配置ZRAM
        cat > /etc/default/zramswap << EOF
# 启用ZRAM
ENABLED=true

# ZRAM设备数量 (默认: CPU核心数)
NUM_DEVICES=$(nproc)

# 每个ZRAM设备的大小 (默认: 物理内存的25%)
# SIZE=25%

# 压缩算法 (默认: lz4)
ALGO=lz4

# 优先级 (默认: 100)
PRIORITY=100
EOF
        
        # 启动ZRAM服务
        systemctl restart zramswap
        echo -e "${GREEN}✓ ZRAM已安装并配置完成${NC}"
    fi
    
    echo -e ""
    
    # 优化系统服务
    echo -e "${PURPLE}5. 优化系统服务${NC}"
    
    # 禁用不需要的服务
    local services_to_disable=("bluetooth" "wifi-country" "raspi-config")
    
    for service in "${services_to_disable[@]}"; do
        if systemctl is-enabled "$service" > /dev/null 2>&1; then
            echo -e "${YELLOW}! 禁用不需要的服务: $service${NC}"
            systemctl disable "$service" > /dev/null 2>&1
            echo -e "${GREEN}✓ 服务 $service 已禁用${NC}"
        fi
    done
    
    echo -e ""
    echo -e "${GREEN}=== 系统优化完成! ===${NC}"
    echo -e "${YELLOW}! 系统需要重启才能应用所有优化${NC}"
    echo -e "请运行: sudo reboot"
}

# 运行完整性能测试
run_benchmark() {
    local duration=${1:-$TEST_DURATION}
    local record_dir=${2:-$RECORD_DIR}
    
    echo -e "${BLUE}=== CinePI 完整性能测试 ===${NC}"
    echo -e "开始时间: $(date)"
    echo -e ""
    
    # 测试系统资源
    test_system_resources $duration
    echo -e ""
    
    # 测试摄像头性能
    test_camera_performance $duration
    echo -e ""
    
    # 测试录制性能
    test_record_performance $RECORD_TEST_DURATION $record_dir
    echo -e ""
    
    echo -e "${BLUE}=== 性能测试总结 ===${NC}"
    echo -e "完成时间: $(date)"
    echo -e ""
    
    # 提供优化建议
    echo -e "${PURPLE}优化建议:${NC}"
    
    # 检查GPU内存
    local gpu_mem=$(grep "gpu_mem" /boot/firmware/config.txt 2>/dev/null | tail -1 | awk -F= '{print $2}')
    if [ -z "$gpu_mem" ] || [ "$gpu_mem" -lt "256" ]; then
        echo -e "- ${YELLOW}增加GPU内存到256MB以提高视频处理性能${NC}"
    fi
    
    # 检查CPU频率调节策略
    if [ -f "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor" ]; then
        local governor=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor)
        if [ "$governor" != "performance" ]; then
            echo -e "- ${YELLOW}启用CPU性能模式以提高录制性能${NC}"
        fi
    fi
    
    # 检查磁盘空间
    local disk_info=$(df -h "$record_dir" | tail -1)
    local available=$(echo $disk_info | awk '{print $4}')
    local available_gb=$(echo $available | sed 's/G//')
    
    if (( $(echo "$available_gb < 10" | bc -l) )); then
        echo -e "- ${YELLOW}磁盘空间不足，请清理或扩展存储${NC}"
    fi
    
    echo -e ""
    echo -e "${GREEN}完整性能测试完成!${NC}"
}

# 主函数
main() {
    local action=""
    local duration=$TEST_DURATION
    local record_dir=$RECORD_DIR
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case "$1" in
            -h|--help)
                show_help
                exit 0
                ;;
            -b|--benchmark)
                action="benchmark"
                shift
                ;;
            -s|--system)
                action="system"
                shift
                ;;
            -c|--camera)
                action="camera"
                shift
                ;;
            -r|--record)
                action="record"
                shift
                ;;
            -o|--optimize)
                action="optimize"
                shift
                ;;
            -d|--directory)
                record_dir="$2"
                shift 2
                ;;
            -t|--time)
                duration="$2"
                shift 2
                ;;
            *)
                echo -e "${RED}未知选项:${NC} $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # 如果没有指定操作，显示帮助
    if [ -z "$action" ]; then
        show_help
        exit 0
    fi
    
    # 执行相应操作
    case "$action" in
        benchmark)
            run_benchmark $duration $record_dir
            ;;
        system)
            test_system_resources $duration
            ;;
        camera)
            test_camera_performance $duration
            ;;
        record)
            test_record_performance $duration $record_dir
            ;;
        optimize)
            apply_optimizations
            ;;
        *)
            echo -e "${RED}未知操作:${NC} $action"
            exit 1
            ;;
    esac
}

# 调用主函数
main "$@"