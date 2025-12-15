// cinepi_preview.cpp
// 基于CinePI SDK和SDL2的摄像头预览应用

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include "src/shared/sdl_helper.h"
#include "src/shared/camera_controller.h"

using namespace cinepi;

// 预览窗口参数
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const std::string WINDOW_TITLE = "CinePI Camera Preview";

// 颜色定义
const Color TEXT_COLOR(255, 255, 255, 255);
const Color BACKGROUND_COLOR(0, 0, 0, 255);
const Color HIGHLIGHT_COLOR(0, 255, 0, 255);
const Color PANEL_COLOR(0, 0, 0, 128);

// 预览应用类
class PreviewApp {
public:
    PreviewApp() : isRunning(false) {
    }
    
    ~PreviewApp() {
        cleanup();
    }
    
    bool initialize() {
        try {
            // 初始化SDL辅助类
            sdlHelper.Initialize();
            
            // 创建窗口
            window = MakeWindow(sdlHelper.CreateWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT));
            
            // 创建渲染器
            renderer = MakeRenderer(sdlHelper.CreateRenderer(window.get()));
            
            // 加载默认字体
            font = MakeFont(sdlHelper.LoadFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16));
            
            // 初始化摄像头控制器
            CameraParams params(WINDOW_WIDTH, WINDOW_HEIGHT, 30, 12);
            cameraController.Initialize(params);
            
            // 创建纹理
            texture = MakeTexture(sdlHelper.CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT));
            
            isRunning = true;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "初始化失败: " << e.what() << std::endl;
            cleanup();
            return false;
        }
    }
    
    void run() {
        try {
            // 启动预览
            cameraController.StartPreview();
            
            SDL_Event event;
            while (isRunning) {
                // 处理事件
                while (SDL_PollEvent(&event)) {
                    handleEvent(event);
                }
                
                // 更新预览
                updatePreview();
                
                // 绘制界面
                render();
                
                // 控制帧率
                std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60fps
            }
            
            // 停止预览
            cameraController.StopPreview();
        } catch (const std::exception& e) {
            std::cerr << "运行时错误: " << e.what() << std::endl;
        }
    }
    
private:
    SDLHelper sdlHelper;
    WindowPtr window;
    RendererPtr renderer;
    TexturePtr texture;
    CameraController cameraController;
    FontPtr font;
    bool isRunning;
    
    // 处理SDL事件
    void handleEvent(SDL_Event& event) {
        switch (event.type) {
            case SDL_QUIT:
                isRunning = false;
                break;
                
            case SDL_KEYDOWN:
                handleKeyPress(event.key.keysym.sym);
                break;
                
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    handleWindowResize(event.window.data1, event.window.data2);
                }
                break;
        }
    }
    
    // 处理键盘按键
    void handleKeyPress(SDL_Keycode key) {
        switch (key) {
            case SDLK_ESCAPE:
                isRunning = false;
                break;
                
            case SDLK_SPACE:
                togglePreview();
                break;
                
            case SDLK_UP:
                cameraController.AdjustExposure(0.5);
                break;
                
            case SDLK_DOWN:
                cameraController.AdjustExposure(-0.5);
                break;
                
            case SDLK_RIGHT:
                cameraController.AdjustISO(50);
                break;
                
            case SDLK_LEFT:
                cameraController.AdjustISO(-50);
                break;
                
            case SDLK_w:
                cameraController.CycleWhiteBalance();
                break;
        }
    }
    
    // 处理窗口大小变化
    void handleWindowResize(int width, int height) {
        // 更新预览分辨率
        cameraController.SetResolution(width, height);
        
        // 重新创建纹理
        texture = MakeTexture(sdlHelper.CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height));
    }
    
    // 切换预览状态
    void togglePreview() {
        if (cameraController.IsPreviewing()) {
            cameraController.StopPreview();
        } else {
            cameraController.StartPreview();
        }
    }
    
    // 更新预览画面
    void updatePreview() {
        if (!cameraController.IsPreviewing()) {
            return;
        }
        
        // 获取预览帧数据
        const uint8_t* frameData = cameraController.GetPreviewFrame();
        if (!frameData) {
            return;
        }
        
        // 更新纹理
        int width = cameraController.GetWidth();
        SDL_UpdateTexture(
            texture.get(),
            nullptr,
            frameData,
            width * 3 // RGB24格式，每行字节数
        );
    }
    
    // 绘制界面
    void render() {
        // 清屏
        SDL_SetRenderDrawColor(renderer.get(), BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, BACKGROUND_COLOR.a);
        SDL_RenderClear(renderer.get());
        
        // 绘制预览画面
        if (cameraController.IsPreviewing() && texture) {
            SDL_RenderCopy(renderer.get(), texture.get(), nullptr, nullptr);
        } else {
            // 绘制占位符
            SDL_SetRenderDrawColor(renderer.get(), 64, 64, 64, 255);
            SDL_Rect rect = {0, 0, cameraController.GetWidth(), cameraController.GetHeight()};
            SDL_RenderFillRect(renderer.get(), &rect);
            sdlHelper.RenderText(renderer.get(), font.get(), "预览已停止", cameraController.GetWidth() / 2 - 50, cameraController.GetHeight() / 2 - 10, TEXT_COLOR);
        }
        
        // 绘制信息面板
        drawInfoPanel();
        
        // 显示渲染结果
        SDL_RenderPresent(renderer.get());
    }
    
    // 绘制信息面板
    void drawInfoPanel() {
        // 绘制半透明背景
        SDL_SetRenderDrawColor(renderer.get(), PANEL_COLOR.r, PANEL_COLOR.g, PANEL_COLOR.b, PANEL_COLOR.a);
        SDL_Rect panelRect = {10, 10, 300, 150};
        SDL_RenderFillRect(renderer.get(), &panelRect);
        
        // 绘制信息文本
        std::string infoText;
        int yPos = 25;
        int lineHeight = 20;
        
        infoText = "CinePI Camera Preview";
        sdlHelper.RenderText(renderer.get(), font.get(), infoText, 20, yPos, TEXT_COLOR);
        yPos += lineHeight;
        
        infoText = "状态: " + std::string(cameraController.IsPreviewing() ? "运行中" : "已停止");
        sdlHelper.RenderText(renderer.get(), font.get(), infoText, 20, yPos, cameraController.IsPreviewing() ? HIGHLIGHT_COLOR : TEXT_COLOR);
        yPos += lineHeight;
        
        infoText = "分辨率: " + std::to_string(cameraController.GetWidth()) + "x" + std::to_string(cameraController.GetHeight());
        sdlHelper.RenderText(renderer.get(), font.get(), infoText, 20, yPos, TEXT_COLOR);
        yPos += lineHeight;
        
        infoText = "帧率: " + std::to_string(cameraController.GetFPS()) + "fps";
        sdlHelper.RenderText(renderer.get(), font.get(), infoText, 20, yPos, TEXT_COLOR);
        yPos += lineHeight;
        
        infoText = "ISO: " + std::to_string(cameraController.GetISO());
        sdlHelper.RenderText(renderer.get(), font.get(), infoText, 20, yPos, TEXT_COLOR);
        yPos += lineHeight;
        
        infoText = "曝光补偿: " + std::to_string(cameraController.GetExposureCompensation());
        sdlHelper.RenderText(renderer.get(), font.get(), infoText, 20, yPos, TEXT_COLOR);
        yPos += lineHeight;
        
        infoText = "白平衡: " + std::to_string(cameraController.GetWhiteBalance()) + "K";
        sdlHelper.RenderText(renderer.get(), font.get(), infoText, 20, yPos, TEXT_COLOR);
        yPos += lineHeight;
        
        // 绘制控制提示
        sdlHelper.RenderText(renderer.get(), font.get(), "空格键: 切换预览", 20, yPos, TEXT_COLOR);
        yPos += lineHeight;
        sdlHelper.RenderText(renderer.get(), font.get(), "方向键: 调整参数", 20, yPos, TEXT_COLOR);
        yPos += lineHeight;
        sdlHelper.RenderText(renderer.get(), font.get(), "ESC: 退出", 20, yPos, TEXT_COLOR);
    }
    
    // 清理资源
    void cleanup() {
        isRunning = false;
        // 资源会通过智能指针自动清理
    }
    
private:
    SDLHelper sdlHelper;
    WindowPtr window;
    RendererPtr renderer;
    TexturePtr texture;
    FontPtr font;
    CameraController cameraController;
    bool isRunning;
};

int main(int argc, char* argv[]) {
    std::cout << "启动CinePI摄像头预览应用..." << std::endl;
    
    // 创建预览应用实例
    PreviewApp app;
    
    // 初始化应用
    if (!app.initialize()) {
        std::cerr << "应用初始化失败！" << std::endl;
        return -1;
    }
    
    // 运行应用
    app.run();
    
    std::cout << "应用已退出" << std::endl;
    return 0;
}