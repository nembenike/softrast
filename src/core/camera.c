#include "camera.h"
#include <math.h>

#define DEG2RAD(x) ((x) * 0.017453292519943295f)

static void camera_process_mouse(Camera* cam, float xoffset, float yoffset) {
    xoffset *= cam->sensitivity;
    yoffset *= cam->sensitivity;

    cam->yaw += xoffset;
    cam->pitch += yoffset;

    if (cam->pitch > 89.0f) cam->pitch = 89.0f;
    if (cam->pitch < -89.0f) cam->pitch = -89.0f;

    camera_update_direction(cam);
}

Camera camera_create(Vec3 position, Vec3 target, Vec3 up, float speed, float sensitivity) {
    Camera cam = {0};
    cam.position = position;
    cam.up = up;
    cam.speed = speed;
    cam.sensitivity = sensitivity;
    cam.yaw = -90.0f;
    cam.pitch = 0.0f;
    camera_update_direction(&cam);
    return cam;
}

Mat4 camera_get_view(Camera* cam) {
    Vec3 forward = vec3_normalize(vec3_sub(cam->target, cam->position));
    Vec3 right   = vec3_normalize(vec3_cross(forward, cam->up));
    Vec3 up      = vec3_cross(right, forward);

    Mat4 view = mat4_identity();
    view.m[0][0] = right.x; view.m[0][1] = right.y; view.m[0][2] = right.z; view.m[0][3] = -vec3_dot(right, cam->position);
    view.m[1][0] = up.x;    view.m[1][1] = up.y;    view.m[1][2] = up.z;    view.m[1][3] = -vec3_dot(up, cam->position);
    view.m[2][0] = -forward.x; view.m[2][1] = -forward.y; view.m[2][2] = -forward.z; view.m[2][3] = vec3_dot(forward, cam->position);
    view.m[3][0] = 0.0f; view.m[3][1] = 0.0f; view.m[3][2] = 0.0f; view.m[3][3] = 1.0f;
    return view;
}

void camera_process_input(Camera* cam, CameraInput input, float delta_time) {
    Vec3 forward = vec3_normalize(vec3_sub(cam->target, cam->position));
    Vec3 right   = vec3_normalize(vec3_cross(forward, cam->up));
    float velocity = cam->speed * delta_time;

    if (input.move_forward) cam->position = vec3_add(cam->position, vec3_scale(forward, velocity));
    if (input.move_back)    cam->position = vec3_sub(cam->position, vec3_scale(forward, velocity));
    if (input.move_left)    cam->position = vec3_sub(cam->position, vec3_scale(right, velocity));
    if (input.move_right)   cam->position = vec3_add(cam->position, vec3_scale(right, velocity));

    camera_process_mouse(cam, input.mouse_dx, input.mouse_dy);
    camera_update_direction(cam);
}

void camera_update_direction(Camera* cam) {
    Vec3 dir;
    dir.x = cosf(DEG2RAD(cam->yaw)) * cosf(DEG2RAD(cam->pitch));
    dir.y = sinf(DEG2RAD(cam->pitch));
    dir.z = sinf(DEG2RAD(cam->yaw)) * cosf(DEG2RAD(cam->pitch));

    cam->target = vec3_add(cam->position, vec3_normalize(dir));
}
