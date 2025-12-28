#ifndef CAMERA_INPUT_H
#define CAMERA_INPUT_H

#include "camera.h"
#include "../platform/input.h"

void camera_handle_input(Camera* cam, Input* input, float delta_seconds);

#endif // CAMERA_INPUT_H
