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
    if (!surface) return;

    memcpy(surface->pixels, frame_buffer, WIDTH * HEIGHT * sizeof(uint32_t));
    SDL_UpdateWindowSurface(g_window);
}

void destroy_display() {
    surface = nullptr;
}
