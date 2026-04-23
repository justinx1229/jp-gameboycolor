#include "display.h"

SDL_Surface *surface = nullptr;
SDL_Window *g_window = nullptr;

bool init_display(SDL_Window *window) {
    g_window = window;
    surface = SDL_GetWindowSurface(window);

    if (!surface) {
        std::cerr << "Surface not there: " << SDL_GetError() << "\n";
        return false;
    }

    return true;
}

void render_display(const uint32_t frame_buffer[HEIGHT][WIDTH]) {
    if (!g_window) return;

    surface = SDL_GetWindowSurface(g_window); // update if resized
    if (!surface) return;

    int win_w = surface->w;
    int win_h = surface->h;

    uint32_t *pixels = (uint32_t*)surface->pixels;

    for (int y = 0; y < win_h; y++) {
        for (int x = 0; x < win_w; x++) {
            int src_x = x * WIDTH / win_w;
            int src_y = y * HEIGHT / win_h;

            pixels[y * surface->w + x] = frame_buffer[src_y][src_x];
        }
    }

    SDL_UpdateWindowSurface(g_window);
}

void destroy_display() {
    surface = nullptr;
}
