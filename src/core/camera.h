#ifndef CAMERA_H
#define CAMERA_H

#include "vec.h"
#include "mat.h"

typedef struct Camera {
    Vec3 position;
    Vec3 target;
    Vec3 up;
    float yaw;
    float pitch;
    float speed;
    float sensitivity;
} Camera;

typedef struct CameraInput {
    int move_forward;
    int move_back;
    int move_left;
    int move_right;
    float mouse_dx;
    float mouse_dy;
} CameraInput;

Camera camera_create(Vec3 position, Vec3 target, Vec3 up, float speed, float sensitivity);
Mat4 camera_get_view(Camera* cam);
void camera_process_input(Camera* cam, CameraInput input, float delta_time);
void camera_update_direction(Camera* cam);

#endif // CAMERA_H