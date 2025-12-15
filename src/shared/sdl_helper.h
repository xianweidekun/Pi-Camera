// sdl_helper.h
// SDL辅助类，用于处理窗口、渲染器和纹理管理

#ifndef SDL_HELPER_H
#define SDL_HELPER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <memory>
#include <stdexcept>

namespace cinepi {

// 颜色定义
struct Color {
    uint8_t r, g, b, a;
    Color(uint8_t red = 255, uint8_t green = 255, uint8_t blue = 255, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

// SDL辅助类
class SDLHelper {
public:
    SDLHelper();
    ~SDLHelper();

    // 初始化SDL
    void Initialize();

    // 创建窗口
    SDL_Window* CreateWindow(const std::string& title, int width, int height, Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    // 创建渲染器
    SDL_Renderer* CreateRenderer(SDL_Window* window, int index = -1, Uint32 flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // 创建纹理
    SDL_Texture* CreateTexture(SDL_Renderer* renderer, Uint32 format, int access, int width, int height);

    // 加载字体
    TTF_Font* LoadFont(const std::string& fontPath, int fontSize);

    // 渲染文本
    void RenderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, const Color& color);

    // 清理资源
    void Cleanup();

private:
    bool initialized_;
    bool ttf_initialized_;
};

// RAII包装器，自动管理SDL资源
using WindowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
using RendererPtr = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;
using TexturePtr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;
using FontPtr = std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)>;

inline WindowPtr MakeWindow(SDL_Window* window) {
    return WindowPtr(window, SDL_DestroyWindow);
}

inline RendererPtr MakeRenderer(SDL_Renderer* renderer) {
    return RendererPtr(renderer, SDL_DestroyRenderer);
}

inline TexturePtr MakeTexture(SDL_Texture* texture) {
    return TexturePtr(texture, SDL_DestroyTexture);
}

inline FontPtr MakeFont(TTF_Font* font) {
    return FontPtr(font, TTF_CloseFont);
}

} // namespace cinepi

#endif // SDL_HELPER_H