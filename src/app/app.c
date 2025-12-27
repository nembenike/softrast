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

    app->camera = camera_create((Vec3){0,0,3}, (Vec3){0,0,0}, (Vec3){0,1,0}, 5.0f, 0.1f);

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
        // back (z = -1)
        {0,1,2}, {0,2,3},
        // front (z = 1)
        {4,5,6}, {4,6,7},
        // bottom (y = -1)
        {0,4,5}, {0,5,1},
        // top (y = 1)
        {3,2,6}, {3,6,7},
        // left (x = -1)
        {0,3,7}, {0,7,4},
        // right (x = 1)
        {1,5,6}, {1,6,2}
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
        Mat4 translation = mat4_translation((Vec3){0,0,-5});
        Mat4 model = mat4_mul(translation, rotation);

        Mat4 view = camera_get_view(&app->camera);

        for (int i=0;i<12;i++) {
            Vec3 world[3], view_space[3], screen[3];
            for (int j=0;j<3;j++) {
                world[j] = mat4_mul_vec3(model, cube[tris[i][j]]);
                view_space[j] = mat4_mul_vec3(view, world[j]);
            }

            int clip_bad = 0;
            for (int j=0;j<3;j++) {
                Vec4 clip = mat4_mul_vec4(proj, (Vec4){view_space[j].x, view_space[j].y, view_space[j].z,1});
                if (fabsf(clip.w) < 1e-6f) { clip_bad = 1; break; }
                Vec3 ndc = { clip.x/clip.w, clip.y/clip.w, clip.z/clip.w };
                ndc.z = (ndc.z+1.0f)*0.5f;
                screen[j] = ndc_to_screen(ndc, app->width, app->height);
            }
            if (clip_bad) continue;

            float area = (screen[1].x - screen[0].x) * (screen[2].y - screen[0].y)
                       - (screen[1].y - screen[0].y) * (screen[2].x - screen[0].x);

            if (fabsf(area) < 1e-6f) continue; // degenerate
            if (area < 0.0f) {
                Vec3 tmp = screen[1];
                screen[1] = screen[2];
                screen[2] = tmp;
            }

            renderer_draw_triangle(app->renderer, screen[0], screen[1], screen[2], 0xFFFF0000);
        }

        renderer_present(app->renderer);

        angle += 0.01f;
        if (angle > 2.0f*3.14159265f) angle -= 2.0f*3.14159265f;

        input_end_frame(&app->input);
    }
}
