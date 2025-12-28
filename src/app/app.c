#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <time.h>
#include <stdio.h>

#include "app.h"
#include "../core/log.h"
#include "../core/mat.h"
#include "../core/vec.h"
#include "../ui/overlay.h"
#include "../core/culling.h"
#include "../scene/scene.h"
#include "../scene/scene_factory.h"
#include "../scene/teapot_scene.h"
#include "../assets/loader.h"
#include "../ui/overlay_helpers.h"
#include "../core/camera_input.h"
#include "../debug/profiler.h"
#include "../core/geom.h"
#include "../assets/pakloader.h"
#include "../assets/objloader.h"
#include "../assets/model.h"

typedef enum {
    APP_STATE_LOADING,
    APP_STATE_RUNNING,
    APP_STATE_EXITING
} AppState;

struct App {
    Window* window;
    Input input;
    Time time;
    Renderer* renderer;
    Camera camera;
    int width;
    int height;
    int wireframe;
    Scene* scene;
    Vec3* loaded_vertices;
    Face* loaded_faces;
    size_t loaded_vertex_count;
    size_t loaded_face_count;
    AppState state;
    int loading_started;
    int loading_done;
    float loading_start_time;
    char loading_message[128];
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

static void draw_cube(Renderer* renderer, Mat4 model, Mat4 view, Mat4 proj, int width, int height) {
    for (int i=0;i<12;i++) {
        Vec3 world[3], screen[3];
        int clip_bad = 0;
        for (int j=0;j<3;j++) {
            world[j] = geom_transform_point(model, cube[tris[i][j]]);
            if (!geom_project_point(view, proj, world[j], width, height, &screen[j], NULL)) { clip_bad = 1; break; }
        }
        if (clip_bad) continue;
        if (geom_triangle_backface_cull(screen)) continue;

        renderer_draw_triangle(renderer, screen[0], screen[1], screen[2], 0xFFFF0000);
    }
}

static void update_angle(float* angle) {
    *angle += 0.01f;
    if (*angle > 2.0f*3.14159265f) *angle -= 2.0f*3.14159265f;
}

static int text_pixel_width(const char* text, int scale) {
    if (!text) return 0;
    int len = (int)strlen(text);
    if (len <= 0) return 0;
    int spacing = scale;
    return len * (3 * scale) + (len - 1) * spacing;
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
        app->wireframe = 1;

        SDL_SetRelativeMouseMode(SDL_TRUE);

        app->loaded_vertices = NULL;
        app->loaded_faces = NULL;
        app->loaded_vertex_count = 0;
        app->loaded_face_count = 0;
        app->state = APP_STATE_LOADING;
        app->loading_started = 0;
        app->loading_done = 0;
        app->loading_start_time = 0.0f;
        app->loading_message[0] = '\0';

    profiler_init();
    return app;
}

void app_destroy(App* app) {
    if (!app) return;
    renderer_destroy(app->renderer);
    window_destroy(app->window);
    scene_manager_destroy();
    if (app->loaded_vertices || app->loaded_faces) obj_free_mesh(app->loaded_vertices, app->loaded_faces);
    free(app);
}

void app_run(App* app) {
    float angle = 0.0f;
    Mat4 proj = mat4_perspective(3.14159265f/3.0f, (float)app->width/app->height, 0.1f, 100.0f);

    uint64_t t_start = 0, t_end = 0;
    double freq = (double)SDL_GetPerformanceFrequency();
    double frame_time = 0.0, draw_time = 0.0, present_time = 0.0;
    int frame_count = 0;

    while (!window_should_close(app->window) && !app->input.quit_requested) {
        t_start = SDL_GetPerformanceCounter();
        input_update(&app->input);
        time_update(&app->time);
        if (app->state == APP_STATE_LOADING) {
            if (!app->loading_started) {
                app->loading_started = 1;
                app->loading_done = 0;
                app->loading_start_time = app->time.total_seconds;
                snprintf(app->loading_message, sizeof(app->loading_message), "Opening %s", "build/assets.pak");
                renderer_clear(app->renderer, 0xFF000000);
                {
                    int w = text_pixel_width(app->loading_message, 4);
                    int x = app->width/2 - w/2;
                    if (x < 0) x = 0;
                    overlay_draw_text(app->renderer, app->loading_message, x, app->height/2 - 8, 4, 0xFFFFFFFF);
                }
                renderer_present(app->renderer);
                input_end_frame(&app->input);
                continue;
            }

            if (!app->loading_done) {
                char msg[128] = {0};
                if (assets_load_model_from_pak("build/assets.pak", "monkey.obj",
                                              &app->loaded_vertices, &app->loaded_faces,
                                              &app->loaded_vertex_count, &app->loaded_face_count,
                                              msg, sizeof(msg))) {
                    snprintf(app->loading_message, sizeof(app->loading_message), "Loaded %s", "monkey.obj");
                    normalize_model(app->loaded_vertices, app->loaded_vertex_count, 1.0f);
                } else {
                    snprintf(app->loading_message, sizeof(app->loading_message), "%s", msg[0] ? msg : "Failed to load model");
                }
                app->loading_done = 1;
            }

            renderer_clear(app->renderer, 0xFF000000);
            overlay_draw_centered_message(app->renderer, app->loading_message, app->width, app->height, 4, 0xFFFFFFFF);
            renderer_present(app->renderer);

            if ((app->time.total_seconds - app->loading_start_time) < 1.0f) {
                input_end_frame(&app->input);
                continue;
            }


            if (app->loaded_vertices && app->loaded_faces) {
                app->scene = scene_factory_create_start_scene(app->loaded_vertices, app->loaded_faces, app->loaded_vertex_count, app->loaded_face_count, app->width, app->height);
                scene_manager_set(app->scene);
            } else {
                app->scene = NULL;
            }

            app->state = APP_STATE_RUNNING;
            continue;
        }

        if (app->state == APP_STATE_RUNNING) {
            camera_handle_input(&app->camera, &app->input, app->time.delta_seconds);

            scene_manager_update(app->time.delta_seconds, &app->input, &app->camera, proj);

            t_start = SDL_GetPerformanceCounter();
            renderer_clear(app->renderer, 0xFF000000);
            scene_manager_render(app->renderer);
            overlay_draw_fps(app->renderer, app->time.delta_seconds);
            t_end = SDL_GetPerformanceCounter();
            profiler_record_draw((double)(t_end - t_start) / freq);

            t_start = SDL_GetPerformanceCounter();
            renderer_present(app->renderer);
            t_end = SDL_GetPerformanceCounter();
            profiler_record_present((double)(t_end - t_start) / freq);

            profiler_frame_end();
        }

        if (app->state == APP_STATE_EXITING) {
            break;
        }

        input_end_frame(&app->input);
        frame_count++;
        t_end = SDL_GetPerformanceCounter();
        frame_time += (double)(t_end - t_start) / freq;

        // Print timing every 300 frames
        if (frame_count % 300 == 0) {
            printf("[PROFILE] avg frame: %.4f ms, draw: %.4f ms, present: %.4f ms\n",
                (frame_time / frame_count) * 1000.0,
                (draw_time / frame_count) * 1000.0,
                (present_time / frame_count) * 1000.0);
        }
    }
}
