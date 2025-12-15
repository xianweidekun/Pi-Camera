#!/bin/bash

# CinePI 存储配置和文件管理脚本

# 默认配置
DEFAULT_RECORD_DIR="/home/pi/cinepi_recordings"
MIN_FREE_SPACE=5  # 最小可用空间 (GB)
MAX_FILE_AGE=30   # 文件最大保留天数
MAX_FILE_COUNT=100  # 最大文件数量

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 显示帮助信息
show_help() {
    echo -e "${BLUE}CinePI 存储配置和文件管理脚本${NC}"
    echo -e "用于管理 CinePI RAW 视频录制的存储配置"
    echo -e ""
    echo -e "选项:"
    echo -e "  -h, --help              显示此帮助信息"
    echo -e "  -s, --setup             设置录制目录"
    echo -e "  -c, --check             检查存储空间"
    echo -e "  -l, --list              列出录制文件"
    echo -e "  -c, --clean             清理旧文件"
    echo -e "  -d, --directory DIR     指定录制目录 (默认: $DEFAULT_RECORD_DIR)"
    echo -e ""
    echo -e "示例:"
    echo -e "  $0 --setup"              # 设置录制目录
    echo -e "  $0 --check"              # 检查存储空间
    echo -e "  $0 --clean"              # 清理旧文件
    echo -e "  $0 --directory /mnt/usb --check"  # 检查指定目录的存储空间
}

# 设置录制目录
setup_record_dir() {
    local record_dir=${1:-$DEFAULT_RECORD_DIR}
    
    echo -e "${BLUE}设置录制目录:${NC} $record_dir"
    
    # 创建目录
    if mkdir -p "$record_dir"; then
        echo -e "${GREEN}✓ 成功创建录制目录:${NC} $record_dir"
    else
        echo -e "${RED}✗ 无法创建录制目录:${NC} $record_dir"
        exit 1
    fi
    
    # 设置权限
    if chmod 755 "$record_dir"; then
        echo -e "${GREEN}✓ 成功设置目录权限:${NC} 755"
    else
        echo -e "${YELLOW}! 无法设置目录权限:${NC} $record_dir"
    fi
    
    # 更新配置文件
    update_config "record_dir" "$record_dir"
    
    echo -e ""
    echo -e "${GREEN}录制目录设置完成!${NC}"
}

# 检查存储空间
check_storage() {
    local record_dir=${1:-$DEFAULT_RECORD_DIR}
    
    echo -e "${BLUE}检查存储空间${NC}"
    echo -e "录制目录: $record_dir"
    echo -e ""
    
    # 检查目录是否存在
    if [ ! -d "$record_dir" ]; then
        echo -e "${RED}✗ 录制目录不存在:${NC} $record_dir"
        echo -e "请先运行 --setup 选项创建目录"
        exit 1
    fi
    
    # 获取磁盘信息
    local disk_info=$(df -h "$record_dir" | tail -1)
    local total=$(echo $disk_info | awk '{print $2}')
    local used=$(echo $disk_info | awk '{print $3}')
    local available=$(echo $disk_info | awk '{print $4}')
    local used_percent=$(echo $disk_info | awk '{print $5}')
    
    echo -e "总容量: $total"
    echo -e "已使用: $used ($used_percent)"
    echo -e "可用空间: $available"
    
    # 检查可用空间
    local available_gb=$(echo $available | sed 's/G//')
    if (( $(echo "$available_gb < $MIN_FREE_SPACE" | bc -l) )); then
        echo -e "${RED}✗ 警告: 可用空间不足${NC}"
        echo -e "  当前可用: $available_gb GB"
        echo -e "  建议至少: $MIN_FREE_SPACE GB"
        return 1
    else
        echo -e "${GREEN}✓ 可用空间充足${NC}"
        return 0
    fi
}

# 列出录制文件
list_files() {
    local record_dir=${1:-$DEFAULT_RECORD_DIR}
    
    echo -e "${BLUE}列出录制文件${NC}"
    echo -e "录制目录: $record_dir"
    echo -e ""
    
    # 检查目录是否存在
    if [ ! -d "$record_dir" ]; then
        echo -e "${RED}✗ 录制目录不存在:${NC} $record_dir"
        return 1
    fi
    
    # 检查是否有文件
    local file_count=$(ls -1 "$record_dir"/*.raw 2>/dev/null | wc -l)
    if [ $file_count -eq 0 ]; then
        echo -e "${YELLOW}没有找到RAW视频文件${NC}"
        return 0
    fi
    
    # 列出文件信息
    echo -e "共找到 $file_count 个RAW视频文件:"
    echo -e ""
    
    # 使用ls命令显示文件详情（按修改时间排序）
    ls -lh "$record_dir"/*.raw 2>/dev/null | sort -k 6,7
    
    echo -e ""
    echo -e "${BLUE}总大小:${NC} $(du -sh "$record_dir" 2>/dev/null | cut -f1)"
    
    return 0
}

# 清理旧文件
clean_files() {
    local record_dir=${1:-$DEFAULT_RECORD_DIR}
    
    echo -e "${BLUE}清理旧文件${NC}"
    echo -e "录制目录: $record_dir"
    echo -e "最大保留天数: $MAX_FILE_AGE 天"
    echo -e "最大文件数量: $MAX_FILE_COUNT 个"
    echo -e ""
    
    # 检查目录是否存在
    if [ ! -d "$record_dir" ]; then
        echo -e "${RED}✗ 录制目录不存在:${NC} $record_dir"
        return 1
    fi
    
    # 清理超过指定天数的文件
    local old_files=$(find "$record_dir" -name "*.raw" -type f -mtime +$MAX_FILE_AGE 2>/dev/null)
    if [ -n "$old_files" ]; then
        echo -e "${YELLOW}找到以下超过 $MAX_FILE_AGE 天的文件:${NC}"
        echo "$old_files"
        echo -e ""
        
        # 询问是否删除
        read -p "是否删除这些文件? (y/N): " answer
        if [[ $answer =~ ^[Yy]$ ]]; then
            echo "$old_files" | xargs rm -f
            echo -e "${GREEN}✓ 成功删除旧文件${NC}"
        else
            echo -e "${YELLOW}! 跳过删除旧文件${NC}"
        fi
    else
        echo -e "${GREEN}✓ 没有超过 $MAX_FILE_AGE 天的文件需要清理${NC}"
    fi
    
    echo -e ""
    
    # 检查文件数量
    local file_count=$(ls -1 "$record_dir"/*.raw 2>/dev/null | wc -l)
    if [ $file_count -gt $MAX_FILE_COUNT ]; then
        echo -e "${YELLOW}文件数量超过限制: $file_count > $MAX_FILE_COUNT${NC}"
        
        # 找到需要删除的最旧文件
        local files_to_delete=$(ls -1t "$record_dir"/*.raw 2>/dev/null | tail -n +$((MAX_FILE_COUNT + 1)))
        local delete_count=$(echo "$files_to_delete" | wc -l)
        
        echo -e "需要删除 $delete_count 个最旧的文件以保持限制"
        echo -e ""
        
        # 询问是否删除
        read -p "是否删除最旧的 $delete_count 个文件? (y/N): " answer
        if [[ $answer =~ ^[Yy]$ ]]; then
            echo "$files_to_delete" | xargs rm -f
            echo -e "${GREEN}✓ 成功删除旧文件${NC}"
        else
            echo -e "${YELLOW}! 跳过删除旧文件${NC}"
        fi
    else
        echo -e "${GREEN}✓ 文件数量在限制范围内: $file_count / $MAX_FILE_COUNT${NC}"
    fi
    
    echo -e ""
    echo -e "${GREEN}清理完成!${NC}"
    
    return 0
}

# 更新配置文件
update_config() {
    local key=$1
    local value=$2
    local config_file="/home/pi/.cinepi_config"
    
    # 创建配置文件目录
    mkdir -p "$(dirname "$config_file")"
    
    # 更新配置文件
    if grep -q "^$key=" "$config_file" 2>/dev/null; then
        # 更新现有配置
        sed -i "s/^$key=.*/$key=$value/" "$config_file"
    else
        # 添加新配置
        echo "$key=$value" >> "$config_file"
    fi
}

# 加载配置文件
load_config() {
    local config_file="/home/pi/.cinepi_config"
    
    if [ -f "$config_file" ]; then
        source "$config_file"
    fi
}

# 主函数
main() {
    local record_dir=$DEFAULT_RECORD_DIR
    local action=""
    
    # 加载配置
    load_config
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case "$1" in
            -h|--help)
                show_help
                exit 0
                ;;
            -s|--setup)
                action="setup"
                shift
                ;;
            -c|--check)
                action="check"
                shift
                ;;
            -l|--list)
                action="list"
                shift
                ;;
            --clean)
                action="clean"
                shift
                ;;
            -d|--directory)
                record_dir="$2"
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
        setup)
            setup_record_dir "$record_dir"
            ;;
        check)
            check_storage "$record_dir"
            ;;
        list)
            list_files "$record_dir"
            ;;
        clean)
            clean_files "$record_dir"
            ;;
        *)
            echo -e "${RED}未知操作:${NC} $action"
            exit 1
            ;;
    esac
}

# 调用主函数
main "$@"