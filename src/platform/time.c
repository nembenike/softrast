#include "time.h"
#include <SDL2/SDL.h>

static uint64_t last_ticks = 0;

void time_init(Time* time) {
    last_ticks = SDL_GetPerformanceCounter();
    time->ticks = last_ticks;
    time->delta_seconds = 0.0f;
    time->total_seconds = 0.0f;
}

void time_update(Time* time) {
    uint64_t now = SDL_GetPerformanceCounter();
    uint64_t freq = SDL_GetPerformanceFrequency();

    uint64_t delta = now - last_ticks;
    last_ticks = now;

    time->delta_seconds = (float)delta / (float)freq;
    time->total_seconds += time->delta_seconds;
    time->ticks = now;
}
