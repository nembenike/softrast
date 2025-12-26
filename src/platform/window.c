#include "window.h"
#include <SDL2/SDL.h>
#include <stdlib.h>

struct Window {
    int width;
    int height;
    const char* title;
    SDL_Window* sdl_window;
    int should_close;
};

Window* window_create(int width, int height, const char* title) {
    Window* w = malloc(sizeof(Window));
    w->width = width;
    w->height = height;
    w->title = title;
    w->should_close = 0;
    w->sdl_window = SDL_CreateWindow(title,
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     width, height, 0);
    return w;
}

void window_destroy(Window* w) {
    if (!w) return;
    SDL_DestroyWindow(w->sdl_window);
    free(w);
}

void window_poll_events(Window* w) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            w->should_close = 1;
        }
    }
}

int window_should_close(Window* w) {
    return w->should_close;
}

void* window_get_handle(Window* w) {
    return w->sdl_window;
}