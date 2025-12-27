#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>

#include "app.h"
#include "../core/log.h"
#include "../core/mat.h"
#include "../core/vec.h"
#include "../ui/overlay.h"
#include "../culling.h"
#include "../scene/teapot_renderer.h"
#include "../core/geom.h"
#include "../assets/pakloader.h"
#include "../assets/objloader.h"

struct App {
    Window* window;
    Input input;
    Time time;
    Renderer* renderer;
    Camera camera;
    int width;
    int height;
    int wireframe;
    struct TeapotRenderer* teapot;
    Vec3* loaded_vertices;
    Face* loaded_faces;
    size_t loaded_vertex_count;
    size_t loaded_face_count;
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

        PakFile pak = {0};
        if (pak_open(&pak, "build/assets.pak")) {
            LOG_INFO("Opened asset pak with %u entries", pak.asset_count);
            AssetEntry* e = pak_find(&pak, "teapot.obj");
            if (e) {
                uint8_t* data = pak_read_asset(&pak, e);
                if (data) {
                    if (obj_parse_from_memory(data, e->size, &app->loaded_vertices, &app->loaded_vertex_count,
                                               &app->loaded_faces, &app->loaded_face_count)) {
                        // parsed successfully
                        LOG_INFO("Loaded teapot mesh: %zu vertices, %zu faces",
                                 app->loaded_vertex_count, app->loaded_face_count);
                    } else {
                        // parse failed: cleanup
                        if (app->loaded_vertices) free(app->loaded_vertices);
                        if (app->loaded_faces) free(app->loaded_faces);
                        app->loaded_vertices = NULL;
                        app->loaded_faces = NULL;
                        app->loaded_vertex_count = 0;
                        app->loaded_face_count = 0;
                    }
                    free(data);
                }
            }
            pak_close(&pak);
        }

        if (app->loaded_vertices && app->loaded_faces) {
            app->teapot = teapot_renderer_create(app->loaded_vertices, app->loaded_faces, app->loaded_vertex_count, app->loaded_face_count);
        } else {
            app->teapot = NULL;
        }

    return app;
}

void app_destroy(App* app) {
    if (!app) return;
    renderer_destroy(app->renderer);
    window_destroy(app->window);
    if (app->teapot) teapot_renderer_destroy(app->teapot);
    // free loaded mesh if any
    if (app->loaded_vertices || app->loaded_faces) obj_free_mesh(app->loaded_vertices, app->loaded_faces);
    free(app);
}

void app_run(App* app) {
    float angle = 0.0f;
    Mat4 proj = mat4_perspective(3.14159265f/3.0f, (float)app->width/app->height, 0.1f, 100.0f);

    while (!window_should_close(app->window) && !app->input.quit_requested) {
        input_update(&app->input);
        time_update(&app->time);

        handle_camera_input(app);

        if (app->input.keyboard.pressed[SDL_SCANCODE_TAB]) {
            app->wireframe = !app->wireframe;
        }

        renderer_clear(app->renderer, 0xFF000000);

        Mat4 model = compute_model_matrix(angle);
        Mat4 view  = camera_get_view(&app->camera);

        int teapot_visible = 0;
        if (app->teapot) {
            teapot_visible = teapot_renderer_update(app->teapot, model, view, proj, app->camera.position, app->width, app->height);
            if (teapot_visible) teapot_renderer_draw(app->teapot, app->renderer, app->wireframe);
        }

        overlay_draw_fps(app->renderer, app->time.delta_seconds);

        renderer_present(app->renderer);

        update_angle(&angle);

        input_end_frame(&app->input);
    }
}
