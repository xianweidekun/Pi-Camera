#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <memory>
#include <cstring>

// 自定义头文件
#include "src/shared/camera_controller.h"
#include "src/shared/sdl_helper.h"

// 定义录制参数
const int PREVIEW_WIDTH = 1280;  // 预览窗口宽度
const int PREVIEW_HEIGHT = 960;  // 预览窗口高度
const int RECORD_WIDTH = 4056;   // IMX477最大分辨率宽度
const int RECORD_HEIGHT = 3040;  // IMX477最大分辨率高度
const int FRAME_RATE = 24;       // 录制帧率
const int BIT_DEPTH = 12;        // 位深度

// 录制状态
enum RecordingStatus {
    IDLE,
    RECORDING,
    STOPPING
};

// 应用程序状态
struct AppState {
    cinepi::SDLHelper sdl_helper;
    cinepi::CameraController camera_controller;
    cinepi::WindowPtr window;
    cinepi::RendererPtr renderer;
    cinepi::TexturePtr texture;
    cinepi::FontPtr font;
    std::unique_ptr<std::ofstream> raw_file;
    RecordingStatus recording_status;
    std::string record_dir;
    std::string current_filename;
    bool running;
    
    // 摄像头参数
    float exposure_compensation;
    int iso;
    int white_balance;
    
    AppState() : recording_status(IDLE), running(true),
                 exposure_compensation(0.0f), iso(100), white_balance(4000),
                 window(nullptr, SDL_DestroyWindow), renderer(nullptr, SDL_DestroyRenderer),
                 texture(nullptr, SDL_DestroyTexture), font(nullptr, TTF_CloseFont) {}
};

// 获取当前时间作为文件名
std::string get_current_time_filename() {
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_time);
    
    std::stringstream ss;
    ss << std::put_time(&local_tm, "cinepi_%Y%m%d_%H%M%S");
    return ss.str();
}

// 创建录制目录
bool create_record_directory(const std::string& dir_path) {
    if (std::system(("mkdir -p " + dir_path).c_str()) != 0) {
        std::cerr << "无法创建录制目录: " << dir_path << std::endl;
        return false;
    }
    return true;
}

// 初始化应用程序
bool init_app(AppState& state, const std::string& record_dir) {
    try {
        // 初始化SDL
        state.sdl_helper.Initialize();
        
        // 创建窗口、渲染器和纹理
        state.window = cinepi::MakeWindow(state.sdl_helper.CreateWindow("CinePI RAW录制", PREVIEW_WIDTH, PREVIEW_HEIGHT));
        state.renderer = cinepi::MakeRenderer(state.sdl_helper.CreateRenderer(state.window.get()));
        state.texture = cinepi::MakeTexture(state.sdl_helper.CreateTexture(state.renderer.get(), SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, PREVIEW_WIDTH, PREVIEW_HEIGHT));
        state.font = cinepi::MakeFont(state.sdl_helper.LoadFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16));
        
        // 初始化摄像头控制器
        cinepi::CameraParams params;
        params.width = RECORD_WIDTH;
        params.height = RECORD_HEIGHT;
        params.fps = FRAME_RATE;
        params.bit_depth = BIT_DEPTH;
        params.exposure_compensation = state.exposure_compensation;
        params.iso = state.iso;
        params.white_balance = state.white_balance;
        
        state.camera_controller.Initialize(params);
        
        // 启动摄像头预览
        state.camera_controller.StartPreview();
        
        // 设置录制目录
        state.record_dir = record_dir;
        if (!create_record_directory(state.record_dir)) {
            return false;
        }
        
        state.recording_status = IDLE;
        state.running = true;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "初始化应用时发生异常: " << e.what() << std::endl;
        return false;
    }
}

// 更新预览窗口
void update_preview(AppState& state) {
    try {
        // 获取最新帧
        const uint8_t* frame_data = state.camera_controller.GetPreviewFrame();
        if (!frame_data) return;
        
        // 清除渲染器
        SDL_SetRenderDrawColor(state.renderer.get(), 0, 0, 0, 255);
        SDL_RenderClear(state.renderer.get());
        
        // 绘制帧
        if (state.texture) {
            SDL_UpdateTexture(state.texture.get(), nullptr, frame_data, PREVIEW_WIDTH * 3);
            SDL_RenderCopy(state.renderer.get(), state.texture.get(), nullptr, nullptr);
        }
        
        // 渲染状态信息
        cinepi::Color white = {255, 255, 255, 255};
        cinepi::Color red = {255, 0, 0, 255};
        
        std::stringstream status_text;
        status_text << "CinePI RAW录制 - " << RECORD_WIDTH << "x" << RECORD_HEIGHT << " " << FRAME_RATE << "fps";
        state.sdl_helper.RenderText(state.renderer.get(), state.font.get(), status_text.str(), 10, 10, white);
        
        // 录制状态
        if (state.recording_status == RECORDING) {
            state.sdl_helper.RenderText(state.renderer.get(), state.font.get(), "录制中...", 10, 30, red);
            state.sdl_helper.RenderText(state.renderer.get(), state.font.get(), "文件: " + state.current_filename, 10, 50, white);
        } else {
            state.sdl_helper.RenderText(state.renderer.get(), state.font.get(), "准备录制", 10, 30, white);
        }
        
        // 参数信息
        std::stringstream params_text;
        params_text << "曝光补偿: " << std::fixed << std::setprecision(1) << state.exposure_compensation;
        state.sdl_helper.RenderText(state.renderer.get(), state.font.get(), params_text.str(), 10, 70, white);
        
        params_text.str("");
        params_text << "ISO: " << state.iso;
        state.sdl_helper.RenderText(state.renderer.get(), state.font.get(), params_text.str(), 10, 90, white);
        
        params_text.str("");
        params_text << "白平衡: " << state.white_balance << "K";
        state.sdl_helper.RenderText(state.renderer.get(), state.font.get(), params_text.str(), 10, 110, white);
        
        // 操作提示
        state.sdl_helper.RenderText(state.renderer.get(), state.font.get(), "空格键: 开始/停止录制", 10, 130, white);
        state.sdl_helper.RenderText(state.renderer.get(), state.font.get(), "上/下箭头: 调整曝光补偿", 10, 150, white);
        state.sdl_helper.RenderText(state.renderer.get(), state.font.get(), "左/右箭头: 调整ISO", 10, 170, white);
        state.sdl_helper.RenderText(state.renderer.get(), state.font.get(), "W键: 循环切换白平衡", 10, 190, white);
        state.sdl_helper.RenderText(state.renderer.get(), state.font.get(), "ESC键: 退出", 10, 210, white);
        
        // 更新屏幕
        SDL_RenderPresent(state.renderer.get());
        
        // 如果正在录制，将帧保存到文件
        if (state.recording_status == RECORDING && state.raw_file) {
            try {
                // 这里应该保存RAW数据，而不是预览数据
                // 由于我们没有实际的RAW流，这里只是演示
                // state.raw_file->write(reinterpret_cast<const char*>(raw_frame_data), raw_frame_size);
            } catch (const std::exception& e) {
                std::cerr << "写入RAW数据时发生异常: " << e.what() << std::endl;
                state.recording_status = STOPPING;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "更新预览时发生异常: " << e.what() << std::endl;
    }
}

// 开始录制
void start_recording(AppState& state) {
    if (state.recording_status != IDLE) return;
    
    try {
        // 生成文件名
        std::string filename = get_current_time_filename();
        state.current_filename = filename + ".raw";
        std::string filepath = state.record_dir + "/" + state.current_filename;
        
        // 打开RAW文件
        state.raw_file = std::make_unique<std::ofstream>(filepath, std::ios::binary);
        if (!state.raw_file->is_open()) {
            throw std::runtime_error("无法打开RAW文件");
        }
        
        // 开始录制
        state.recording_status = RECORDING;
        std::cout << "开始录制RAW视频: " << filepath << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "开始录制时发生异常: " << e.what() << std::endl;
        state.raw_file.reset();
    }
}

// 停止录制
void stop_recording(AppState& state) {
    if (state.recording_status != RECORDING) return;
    
    state.recording_status = STOPPING;
    
    if (state.raw_file) {
        try {
            state.raw_file->close();
            std::cout << "停止录制RAW视频: " << state.current_filename << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "停止录制时发生异常: " << e.what() << std::endl;
        }
        state.raw_file.reset();
    }
    
    state.recording_status = IDLE;
}

// 处理键盘事件
void handle_keyboard(AppState& state, SDL_Event& event) {
    switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            // 退出应用
            state.running = false;
            break;
            
        case SDLK_SPACE:
            // 开始/停止录制
            if (state.recording_status == IDLE) {
                start_recording(state);
            } else if (state.recording_status == RECORDING) {
                stop_recording(state);
            }
            break;
            
        case SDLK_UP:
            // 增加曝光补偿
            state.exposure_compensation += 0.1f;
            state.camera_controller.SetExposureCompensation(state.exposure_compensation);
            break;
            
        case SDLK_DOWN:
            // 减少曝光补偿
            state.exposure_compensation -= 0.1f;
            state.camera_controller.SetExposureCompensation(state.exposure_compensation);
            break;
            
        case SDLK_RIGHT:
            // 增加ISO
            state.iso += 10;
            if (state.iso > 3200) state.iso = 3200;
            state.camera_controller.SetISO(state.iso);
            break;
            
        case SDLK_LEFT:
            // 减少ISO
            state.iso -= 10;
            if (state.iso < 100) state.iso = 100;
            state.camera_controller.SetISO(state.iso);
            break;
            
        case SDLK_w:
            // 循环切换白平衡
            state.white_balance += 500;
            if (state.white_balance > 6500) state.white_balance = 3000;
            state.camera_controller.SetWhiteBalance(state.white_balance);
            break;
            
        default:
            break;
    }
}

int main(int argc, char* argv[]) {
    // 默认录制目录
    std::string record_dir = "/home/pi/cinepi_recordings";
    
    // 检查命令行参数
    if (argc > 1) {
        record_dir = argv[1];
    }
    
    // 创建应用状态
    AppState state;
    
    // 初始化应用
    if (!init_app(state, record_dir)) {
        std::cerr << "初始化应用失败" << std::endl;
        return 1;
    }
    
    // 主循环
    while (state.running) {
        SDL_Event event;
        
        // 处理事件
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state.running = false;
                    break;
                    
                case SDL_KEYDOWN:
                    handle_keyboard(state, event);
                    break;
                    
                default:
                    break;
            }
        }
        
        // 更新预览
        update_preview(state);
        
        // 控制帧率
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FRAME_RATE));
    }
    
    // 如果正在录制，停止录制
    if (state.recording_status == RECORDING) {
        stop_recording(state);
    }
    
    // 停止摄像头预览
    state.camera_controller.StopPreview();
    
    return 0;
}