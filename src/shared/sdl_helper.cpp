// sdl_helper.cpp
// SDL辅助类实现

#include "sdl_helper.h"
#include <iostream>

namespace cinepi {

SDLHelper::SDLHelper() : initialized_(false), ttf_initialized_(false) {
}

SDLHelper::~SDLHelper() {
    Cleanup();
}

void SDLHelper::Initialize() {
    if (initialized_) {
        return;
    }

    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0) {
        throw std::runtime_error("SDL初始化失败: " + std::string(SDL_GetError()));
    }
    initialized_ = true;

    // 初始化SDL_ttf
    if (TTF_Init() < 0) {
        SDL_Quit();
        throw std::runtime_error("SDL_ttf初始化失败: " + std::string(TTF_GetError()));
    }
    ttf_initialized_ = true;
}

SDL_Window* SDLHelper::CreateWindow(const std::string& title, int width, int height, Uint32 flags) {
    if (!initialized_) {
        Initialize();
    }

    SDL_Window* window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        flags
    );

    if (!window) {
        throw std::runtime_error("窗口创建失败: " + std::string(SDL_GetError()));
    }

    return window;
}

SDL_Renderer* SDLHelper::CreateRenderer(SDL_Window* window, int index, Uint32 flags) {
    if (!window) {
        throw std::runtime_error("无效的窗口指针");
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, index, flags);
    if (!renderer) {
        throw std::runtime_error("渲染器创建失败: " + std::string(SDL_GetError()));
    }

    return renderer;
}

SDL_Texture* SDLHelper::CreateTexture(SDL_Renderer* renderer, Uint32 format, int access, int width, int height) {
    if (!renderer) {
        throw std::runtime_error("无效的渲染器指针");
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, format, access, width, height);
    if (!texture) {
        throw std::runtime_error("纹理创建失败: " + std::string(SDL_GetError()));
    }

    return texture;
}

TTF_Font* SDLHelper::LoadFont(const std::string& fontPath, int fontSize) {
    if (!ttf_initialized_) {
        Initialize();
    }

    TTF_Font* font = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!font) {
        // 尝试加载备选字体
        const char* alternateFonts[] = {
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/freefont/FreeSans.ttf"
        };

        for (const char* altFont : alternateFonts) {
            font = TTF_OpenFont(altFont, fontSize);
            if (font) {
                std::cerr << "警告: 无法加载指定字体，使用备选字体: " << altFont << std::endl;
                break;
            }
        }

        if (!font) {
            throw std::runtime_error("无法加载任何字体: " + std::string(TTF_GetError()));
        }
    }

    return font;
}

void SDLHelper::RenderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, const Color& color) {
    if (!renderer || !font) {
        return;
    }

    SDL_Color sdlColor = { color.r, color.g, color.b, color.a };
    
    SDL_Surface* textSurface = TTF_RenderUTF8_Solid(font, text.c_str(), sdlColor);
    if (!textSurface) {
        std::cerr << "文本表面创建失败: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cerr << "文本纹理创建失败: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect textRect = { x, y, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

void SDLHelper::Cleanup() {
    if (ttf_initialized_) {
        TTF_Quit();
        ttf_initialized_ = false;
    }

    if (initialized_) {
        SDL_Quit();
        initialized_ = false;
    }
}

} // namespace cinepi