#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include <stdint.h>

#define INPUT_MAX_KEYS 512
#define INPUT_MAX_MOUSE_BUTTONS 5

typedef struct Keyboard {
    uint8_t down[INPUT_MAX_KEYS];
    uint8_t pressed[INPUT_MAX_KEYS];
    uint8_t released[INPUT_MAX_KEYS];
} Keyboard;

typedef struct Mouse {
    int x;
    int y;
    uint8_t buttons[INPUT_MAX_MOUSE_BUTTONS];
    uint8_t buttons_pressed[INPUT_MAX_MOUSE_BUTTONS];
    uint8_t buttons_released[INPUT_MAX_MOUSE_BUTTONS];
} Mouse;

typedef struct Input {
    Keyboard keyboard;
    Mouse mouse;
    int quit_requested;
} Input;

void input_begin_frame(Input* input);
void input_process_event(Input* input, SDL_Event* event);
void input_update(Input* input);
void input_end_frame(Input* input);

#endif // INPUT_H
