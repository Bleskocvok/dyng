/*
   Copyright 2020 František Bráblík

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#pragma once

#include <SDL2/SDL.h>

#include <memory>
#include <stdexcept>
#include <string> // string literals

namespace demo {

namespace sdl {

/// RAII wrapper for sdl initialization.
class sdl_init {
public:
    sdl_init() {
        using namespace std::literals;
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw std::runtime_error("could not initialize video: "s + SDL_GetError());
        }
    }

    sdl_init(const sdl_init&) = delete;
    sdl_init& operator=(const sdl_init&) = delete;

    ~sdl_init() {
        SDL_Quit();
    }
};

namespace types {

struct deleter {
    void operator()(SDL_Renderer* renderer) { SDL_DestroyRenderer(renderer); }
    void operator()(SDL_Window* window) { SDL_DestroyWindow(window); }
    void operator()(SDL_Texture* texture) { SDL_DestroyTexture(texture); }
    void operator()(SDL_Surface* surface) { SDL_FreeSurface(surface); }
};

using renderer = std::unique_ptr<SDL_Renderer, deleter>;
using window = std::unique_ptr<SDL_Window, deleter>;
using texture = std::unique_ptr<SDL_Texture, deleter>;
using Surface = std::unique_ptr<SDL_Surface, deleter>;

} // namespace types

class window_renderer {
public:
    window_renderer(int width, int height, const char* title) {
        using namespace std::string_literals;
        m_window.reset(SDL_CreateWindow(
                title,
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                width,
                height,
                0));
        if (!m_window) {
            throw std::runtime_error("could not create window: "s + SDL_GetError());
        }
        // set up renderer with vsync
        m_renderer.reset(SDL_CreateRenderer(m_window.get(), -1, SDL_RENDERER_PRESENTVSYNC));
        if (!m_renderer) {
            throw std::runtime_error("could not create renderer: "s + SDL_GetError());
        }
    }

    void clear() {
        SDL_SetRenderDrawColor(m_renderer.get(), 255, 255, 255, 255);
        SDL_RenderClear(m_renderer.get());
    }

    void draw(const types::texture& texture, int x, int y, int w, int h) {
        SDL_Rect rect;
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
        SDL_RenderCopy(m_renderer.get(), texture.get(), nullptr, &rect);
    }

    void draw_rotated(const types::texture& texture
                , int x
                , int y
                , int w
                , int h
                , float angle
                , float alpha) {
        SDL_Rect rect;
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
        SDL_SetTextureAlphaMod(texture.get(), alpha * 255);
        SDL_RenderCopyEx(m_renderer.get(),
                texture.get(),
                nullptr,
                &rect,
                angle,
                nullptr,
                SDL_FLIP_NONE);
    }

    void screenshot(const std::string& filename) const {
        using namespace std::string_literals;
        int w, h;
        SDL_GetRendererOutputSize(m_renderer.get(), &w, &h);
        types::Surface surface(create_surface(w, h));
        if (SDL_RenderReadPixels(
                m_renderer.get(),
                nullptr,
                surface->format->format,
                surface->pixels,
                surface->pitch) != 0) {
            throw std::runtime_error("could not capture screenshot: "s + SDL_GetError());
        }
        if (SDL_SaveBMP(surface.get(), filename.c_str()) != 0) {
            throw std::runtime_error("could not save screenshot: "s + SDL_GetError());
        }
    }

    void render() {
        SDL_RenderPresent(m_renderer.get());
    }

    types::texture make_black_pixel() const {
        types::Surface surface(create_surface(1, 1));
        SDL_LockSurface(surface.get());
        set_pixel(surface, 0, 0, 0, 0, 0, 255);
        SDL_UnlockSurface(surface.get());
        types::texture result(SDL_CreateTextureFromSurface(m_renderer.get(), surface.get()));
        return result;
    }

    types::texture make_circle(int img_size) const {
        types::Surface surface(create_surface(img_size, img_size));
        SDL_LockSurface(surface.get());
        float middle = img_size * 0.5f;
        for (int y = 0; y < img_size; ++y) {
            for (int x = 0; x < img_size; ++x) {
                float xdiff = x - middle;
                float ydiff = y - middle;
                if (xdiff * xdiff + ydiff * ydiff <= img_size * img_size * 0.25f) {
                    set_pixel(surface, x, y, 0, 0, 0, 255);
                } else {
                    set_pixel(surface, x, y, 0, 0, 0, 0);
                }
            }
        }
        SDL_UnlockSurface(surface.get());
        types::texture result(SDL_CreateTextureFromSurface(m_renderer.get(), surface.get()));
        return result;
    }

    void draw_line(int x1, int y1, int x2, int y2, float visibility) {
        int grey = 255 - (visibility * 255);
        SDL_SetRenderDrawColor(m_renderer.get(), grey, grey, grey, 255);
        SDL_RenderDrawLine(m_renderer.get(), x1, y1, x2, y2);
    }

private:
    types::window m_window;
    types::renderer m_renderer;

    SDL_Surface* create_surface(int width, int height) const {
        // need to check whether big or little endian is used
        // https://wiki.libsdl.org/SDL_CreateRGBSurface
        #if SDL_BYTEORDER == SDL_BIG_ENDIAN
            return SDL_CreateRGBSurface(0, width, height, 32,
                    0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
        #else
            return SDL_CreateRGBSurface(0, width, height, 32,
                    0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
        #endif
    }

    void set_pixel(types::Surface& surface, int x, int y, int r, int g, int b, int a) const {
        static_cast<Uint32*>(surface->pixels)[x + y * surface->w]
                = SDL_MapRGBA(surface->format, r, g, b, a);
    }
};

class timer {
public:
    timer(): m_last(SDL_GetTicks()) {}

    void reset() {
        m_last = SDL_GetTicks();
    }

    unsigned time() const {
        return SDL_GetTicks() - m_last;
    }

    void wait(unsigned ms) {
        SDL_Delay(ms);
    }

private:
    unsigned m_last;
};

} // namespace sdl

} // namespace demo
