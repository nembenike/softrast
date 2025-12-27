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

static const Vec3 cube[8] = {
    {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
    {-1,-1,1}, {1,-1,1}, {1,1,1}, {-1,1,1}
};

static const int tris[12][3] = {
    {0,1,2}, {0,2,3},  // back
    {4,5,6}, {4,6,7},  // front
    {0,4,5}, {0,5,1},  // bottom
    {3,2,6}, {3,6,7},  // top
    {0,3,7}, {0,7,4},  // left
    {1,5,6}, {1,6,2}   // right
};

static void handle_camera_input(App* app) {
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
}

static Mat4 compute_model_matrix(float angle) {
    Mat4 rotation = mat4_rotation_y(angle);
    Mat4 translation = mat4_translation((Vec3){0,0,-5});
    return mat4_mul(translation, rotation);
}

static Vec3 transform_vertex(Mat4 m, Vec3 v) {
    return mat4_mul_vec3(m, v);
}

static Vec3 project_vertex(Mat4 view, Mat4 proj, Vec3 world, int width, int height) {
    Vec3 view_space = mat4_mul_vec3(view, world);
    Vec4 clip = mat4_mul_vec4(proj, (Vec4){view_space.x, view_space.y, view_space.z, 1.0f});
    if (fabsf(clip.w) < 1e-6f) return (Vec3){INFINITY, INFINITY, INFINITY};
    Vec3 ndc = { clip.x/clip.w, clip.y/clip.w, clip.z/clip.w };
    ndc.z = (ndc.z+1.0f)*0.5f;
    return ndc_to_screen(ndc, width, height);
}

static int backface_cull(Vec3 screen[3]) {
    float area = (screen[1].x - screen[0].x) * (screen[2].y - screen[0].y)
               - (screen[1].y - screen[0].y) * (screen[2].x - screen[0].x);
    if (fabsf(area) < 1e-6f) return 1; // degenerate
    if (area < 0.0f) { // flip triangle winding
        Vec3 tmp = screen[1];
        screen[1] = screen[2];
        screen[2] = tmp;
    }
    return 0;
}

static void draw_cube(Renderer* renderer, Mat4 model, Mat4 view, Mat4 proj, int width, int height) {
    for (int i=0;i<12;i++) {
        Vec3 world[3], screen[3];
        int clip_bad = 0;
        for (int j=0;j<3;j++) {
            world[j] = transform_vertex(model, cube[tris[i][j]]);
            screen[j] = project_vertex(view, proj, world[j], width, height);
            if (screen[j].x == INFINITY) { clip_bad = 1; break; }
        }
        if (clip_bad) continue;
        if (backface_cull(screen)) continue;

        renderer_draw_triangle(renderer, screen[0], screen[1], screen[2], 0xFFFF0000);
    }
}

static void update_angle(float* angle) {
    *angle += 0.01f;
    if (*angle > 2.0f*3.14159265f) *angle -= 2.0f*3.14159265f;
}

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
    Mat4 proj = mat4_perspective(3.14159265f/3.0f, (float)app->width/app->height, 0.1f, 100.0f);

    while (!window_should_close(app->window) && !app->input.quit_requested) {
        input_update(&app->input);
        time_update(&app->time);

        handle_camera_input(app);

        renderer_clear(app->renderer, 0xFF000000);

        Mat4 model = compute_model_matrix(angle);
        Mat4 view  = camera_get_view(&app->camera);

        draw_cube(app->renderer, model, view, proj, app->width, app->height);

        renderer_present(app->renderer);

        update_angle(&angle);

        input_end_frame(&app->input);
    }
}
