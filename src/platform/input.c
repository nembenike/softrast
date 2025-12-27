#include "input.h"
#include <string.h>

void input_process_event(Input* input, SDL_Event* event) {
    switch (event->type) {
        case SDL_QUIT: input->quit_requested = 1; break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            int key = event->key.keysym.scancode;
            if (key >= 0 && key < INPUT_MAX_KEYS) {
                if (event->type == SDL_KEYDOWN) {
                    if (!input->keyboard.down[key]) input->keyboard.pressed[key] = 1;
                    input->keyboard.down[key] = 1;

                    if (key == SDL_SCANCODE_ESCAPE) input->quit_requested = 1;
                } else {
                    input->keyboard.down[key] = 0;
                    input->keyboard.released[key] = 1;
                }
            }
            break;
        }
        case SDL_MOUSEMOTION:
            input->mouse.x = event->motion.x;
            input->mouse.y = event->motion.y;
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            int button = event->button.button;
            if (button >= 1 && button <= INPUT_MAX_MOUSE_BUTTONS) {
                button -= 1;
                if (event->type == SDL_MOUSEBUTTONDOWN) {
                    if (!input->mouse.buttons[button]) input->mouse.buttons_pressed[button] = 1;
                    input->mouse.buttons[button] = 1;
                } else {
                    input->mouse.buttons[button] = 0;
                    input->mouse.buttons_released[button] = 1;
                }
            }
            break;
        }
        default: break;
    }
}

void input_update(Input* input) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        input_process_event(input, &event);
    }
}

void input_end_frame(Input* input) {
    memset(input->keyboard.pressed, 0, sizeof(input->keyboard.pressed));
    memset(input->keyboard.released, 0, sizeof(input->keyboard.released));
    memset(input->mouse.buttons_pressed, 0, sizeof(input->mouse.buttons_pressed));
    memset(input->mouse.buttons_released, 0, sizeof(input->mouse.buttons_released));
}
