#include "camera_input.h"
#include <SDL2/SDL.h>

void camera_handle_input(Camera* cam, Input* input, float delta_seconds) {
    int dx = 0, dy = 0;
    SDL_GetRelativeMouseState(&dx, &dy);

    CameraInput cam_input = {0};
    cam_input.move_forward = input->keyboard.down[SDL_SCANCODE_W];
    cam_input.move_back    = input->keyboard.down[SDL_SCANCODE_S];
    cam_input.move_left    = input->keyboard.down[SDL_SCANCODE_A];
    cam_input.move_right   = input->keyboard.down[SDL_SCANCODE_D];
    cam_input.mouse_dx     = (float)dx;
    cam_input.mouse_dy     = (float)-dy;

    camera_process_input(cam, cam_input, delta_seconds);
}
