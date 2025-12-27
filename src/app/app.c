#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>

#include "app.h"
#include "../core/mat.h"
#include "../core/vec.h"

struct App {
    Window* window;
    Input input;
    Time time;
    Renderer* renderer;
    Camera camera;
    int width;
    int height;
};

App* app_create(int width, int height, const char* title) {
    App* app = malloc(sizeof(App));
    if (!app) return NULL;
    memset(app, 0, sizeof(App));

    app->window = window_create(width, height, title);
    if (!app->window) { free(app); return NULL; }

    app->renderer = renderer_create(width, height, window_get_handle(app->window));
    if (!app->renderer) { window_destroy(app->window); free(app); return NULL; }

    time_init(&app->time);
    memset(&app->input, 0, sizeof(Input));

    app->camera = camera_create((Vec3){0,0,0}, (Vec3){0,0,1}, (Vec3){0,1,0}, 5.0f, 0.1f);

    app->width = width;
    app->height = height;

    SDL_SetRelativeMouseMode(SDL_TRUE);

    return app;
}

void app_destroy(App* app) {
    if (!app) return;
    renderer_destroy(app->renderer);
    window_destroy(app->window);
    free(app);
}

void app_run(App* app) {
    float angle = 0.0f;

    Vec3 cube[8] = {
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
        {-1,-1,1}, {1,-1,1}, {1,1,1}, {-1,1,1}
    };

    int tris[12][3] = {
        {0,2,1},{0,3,2},
        {4,5,6},{4,6,7},
        {0,1,5},{0,5,4},
        {3,7,6},{3,6,2},
        {0,4,7},{0,7,3},
        {1,2,6},{1,6,5}
    };

    Mat4 proj = mat4_perspective(3.14159265f/3.0f, (float)app->width/app->height, 0.1f, 100.0f);

    while (!window_should_close(app->window) && !app->input.quit_requested) {
        input_update(&app->input);
        time_update(&app->time);

        int dx, dy;
        SDL_GetRelativeMouseState(&dx, &dy);

        CameraInput cam_input = {0};
        cam_input.move_forward = app->input.keyboard.down[SDL_SCANCODE_W];
        cam_input.move_back    = app->input.keyboard.down[SDL_SCANCODE_S];
        cam_input.move_left    = app->input.keyboard.down[SDL_SCANCODE_A];
        cam_input.move_right   = app->input.keyboard.down[SDL_SCANCODE_D];
        cam_input.mouse_dx     = (float)dx;
        cam_input.mouse_dy     = (float)-dy;

        camera_process_input(&app->camera, cam_input, app->time.delta_seconds);

        renderer_clear(app->renderer, 0xFF000000);

        Mat4 rotation = mat4_rotation_y(angle);
        Mat4 translation = mat4_translation((Vec3){0,0,5});
        Mat4 model = mat4_mul(rotation, translation);

        Mat4 view = camera_get_view(&app->camera);

        for (int i=0;i<12;i++) {
            Vec3 world[3], view_space[3], screen[3];
            for (int j=0;j<3;j++) {
                world[j] = mat4_mul_vec3(model, cube[tris[i][j]]);
                view_space[j] = mat4_mul_vec3(view, world[j]);
            }

            Vec3 e1 = vec3_sub(view_space[1], view_space[0]);
            Vec3 e2 = vec3_sub(view_space[2], view_space[0]);
            Vec3 normal = vec3_cross(e1, e2);
            if (normal.z >= 0.0f) continue;

            for (int j=0;j<3;j++) {
                Vec4 clip = mat4_mul_vec4(proj, (Vec4){view_space[j].x, view_space[j].y, view_space[j].z,1});
                if (fabsf(clip.w) < 1e-6f) continue;
                Vec3 ndc = { clip.x/clip.w, clip.y/clip.w, clip.z/clip.w };
                ndc.z = (ndc.z+1.0f)*0.5f;
                screen[j] = ndc_to_screen(ndc, app->width, app->height);
            }

            renderer_draw_triangle(app->renderer, screen[0], screen[1], screen[2], 0xFFFF0000);
        }

        renderer_present(app->renderer);

        angle += 0.01f;
        if (angle > 2.0f*3.14159265f) angle -= 2.0f*3.14159265f;

        input_end_frame(&app->input);
    }
}
