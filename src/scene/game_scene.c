#include "game_scene.h"
#include "scene/game_object.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "core/mat.h"
#include "core/camera.h"
#include "core/geom.h"
#include "renderer/renderer.h"
#include "core/math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    GameObject** objects;
    size_t count;

    Vec3* ground_vertices;
    Face* ground_faces;
    size_t ground_vcount;
    size_t ground_fcount;

    Vec3* player_vertices;
    Face* player_faces;
    size_t player_vcount;
    size_t player_fcount;

    TeapotRenderer* player_renderer;
    TeapotRenderer* ground_renderer;

    Mat4 proj, view;
    Vec3 camera_pos;
    int width, height;

    size_t player_index;
    float player_speed;
    float player_yaw;
    float model_forward_offset;
    float rotation_speed;

    Vec3 player_pos;
} GameSceneData;

static void make_plane(Vec3** v, Face** f, size_t* vc, size_t* fc, float w, float d, float y) {
    *vc = 4;
    *fc = 2;
    *v = malloc(sizeof(Vec3) * 4);
    *f = malloc(sizeof(Face) * 2);

    (*v)[0] = (Vec3){-w/2, y, -d/2};
    (*v)[1] = (Vec3){ w/2, y, -d/2};
    (*v)[2] = (Vec3){ w/2, y,  d/2};
    (*v)[3] = (Vec3){-w/2, y,  d/2};

    (*f)[0] = (Face){0,1,2};
    (*f)[1] = (Face){0,2,3};
}

static void game_scene_init(Scene* scene) {
    GameSceneData* d = scene->data;

    d->ground_renderer = teapot_renderer_create(
        d->ground_vertices, d->ground_faces,
        d->ground_vcount, d->ground_fcount
    );

    d->player_renderer = teapot_renderer_create(
        d->player_vertices, d->player_faces,
        d->player_vcount, d->player_fcount
    );

    d->count = 4;
    d->objects = calloc(d->count, sizeof(GameObject*));

    d->player_pos = (Vec3){0, 0.5f, -4};

    d->objects[0] = game_object_create_mesh(d->ground_renderer, mat4_identity());
    d->objects[3] = game_object_create_mesh(
        d->player_renderer,
        mat4_translation(d->player_pos)
    );

    for (size_t i = 0; i < d->count; ++i) {
        if (!d->objects[i]) continue;
        d->objects[i]->visible = 0;
    }
    if (d->objects[3]) d->objects[3]->visible = 1;

    d->player_index = 3;
    d->player_speed = 3.0f;
    d->player_yaw = 0.0f;

    d->model_forward_offset = 0.0f;
    d->rotation_speed = 8.0f;
}

static void game_scene_update(Scene* scene, float dt, Input* input, Camera* camera, Mat4 proj) {
    GameSceneData* d = scene->data;

    d->proj = proj;
    d->view = camera_get_view(camera);
    d->camera_pos = camera->position;

    GameObject* player = d->objects[d->player_index];
    if (!player) return;

    Vec3 cam_fwd = vec3_sub(camera->target, camera->position);
    cam_fwd.y = 0.0f;
    cam_fwd = vec3_normalize(cam_fwd);

    Vec3 cam_right = vec3_normalize(vec3_cross(cam_fwd, (Vec3){0,1,0}));

    Vec3 input_move = {0,0,0};
    if (input->keyboard.down[SDL_SCANCODE_W]) input_move.z += 1;
    if (input->keyboard.down[SDL_SCANCODE_S]) input_move.z -= 1;
    if (input->keyboard.down[SDL_SCANCODE_A]) input_move.x -= 1;
    if (input->keyboard.down[SDL_SCANCODE_D]) input_move.x += 1;

    if (input_move.x || input_move.z) {
        Vec3 move = vec3_add(
            vec3_scale(cam_fwd, input_move.z),
            vec3_scale(cam_right, input_move.x)
        );
        move = vec3_normalize(move);

        d->player_pos = vec3_add(
            d->player_pos,
            vec3_scale(move, d->player_speed * dt)
        );

        float target_yaw = atan2f(move.x, move.z) + d->model_forward_offset;

        float diff = target_yaw - d->player_yaw;
        while (diff > M_PI) diff -= 2 * M_PI;
        while (diff < -M_PI) diff += 2 * M_PI;

        float step = d->rotation_speed * dt;
        if (diff > step) diff = step;
        if (diff < -step) diff = -step;

        d->player_yaw += diff;
    }

    player->model = mat4_mul(
        mat4_translation(d->player_pos),
        mat4_rotation_y(d->player_yaw)
    );

    camera->target = (Vec3){
        d->player_pos.x,
        d->player_pos.y + 1.0f,
        d->player_pos.z
    };
    camera->distance = 6.0f;

    for (size_t i = 0; i < d->count; ++i) {
        GameObject* go = d->objects[i];
        if (!go || !go->visible) continue;

        teapot_renderer_update(
            go->mesh,
            go->model,
            d->view,
            d->proj,
            d->camera_pos,
            d->width,
            d->height
        );
    }
}

static void game_scene_render(Scene* scene, Renderer* r) {
    GameSceneData* d = scene->data;

    int tiles_x = 20;
    int tiles_z = 20;
    float tile_size = 1.0f;
    float half_w = (tiles_x * tile_size) * 0.5f;
    float half_d = (tiles_z * tile_size) * 0.5f;

    for (int iz = 0; iz < tiles_z; ++iz) {
        for (int ix = 0; ix < tiles_x; ++ix) {
            float x0 = ix * tile_size - half_w;
            float z0 = iz * tile_size - half_d;
            float x1 = x0 + tile_size;
            float z1 = z0 + tile_size;

            Vec3 w0 = {x0, 0.0f, z0};
            Vec3 w1 = {x1, 0.0f, z0};
            Vec3 w2 = {x1, 0.0f, z1};
            Vec3 w3 = {x0, 0.0f, z1};

            Vec3 s0, s1, s2, s3;
            if (!geom_project_point(d->view, d->proj, w0, d->width, d->height, &s0, NULL)) continue;
            if (!geom_project_point(d->view, d->proj, w1, d->width, d->height, &s1, NULL)) continue;
            if (!geom_project_point(d->view, d->proj, w2, d->width, d->height, &s2, NULL)) continue;
            if (!geom_project_point(d->view, d->proj, w3, d->width, d->height, &s3, NULL)) continue;

            if (geom_triangle_backface_cull((Vec3[]){s0, s1, s2})) continue;

            uint32_t color = ((ix + iz) & 1) ? 0xFF404040 : 0xFF202020;
            renderer_draw_triangle(r, s0, s1, s2, color);
            renderer_draw_triangle(r, s0, s2, s3, color);
        }
    }

    for (size_t i = 0; i < d->count; ++i) {
        GameObject* go = d->objects[i];
        if (!go || !go->visible) continue;
        if (go->type == GO_TYPE_MESH && go->mesh) {
            teapot_renderer_draw(go->mesh, r, 0);
        }
    }
}

static void game_scene_destroy(Scene* scene) {
    GameSceneData* d = scene->data;

    for (size_t i = 0; i < d->count; ++i)
        if (d->objects[i]) game_object_destroy(d->objects[i]);

    free(d->objects);
    teapot_renderer_destroy(d->ground_renderer);
    teapot_renderer_destroy(d->player_renderer);
    free(d->ground_vertices);
    free(d->ground_faces);
    free(d->player_vertices);
    free(d->player_faces);
    free(d);
}

static SceneVTable vtable = {
    .init = game_scene_init,
    .update = game_scene_update,
    .render = game_scene_render,
    .destroy = game_scene_destroy
};

Scene* game_scene_create(
    const Vec3* verts,
    const Face* faces,
    size_t vc,
    size_t fc,
    int w,
    int h
) {
    Scene* s = malloc(sizeof(Scene));
    GameSceneData* d = calloc(1, sizeof(GameSceneData));

    d->width = w;
    d->height = h;

    d->player_vertices = malloc(sizeof(Vec3) * vc);
    d->player_faces = malloc(sizeof(Face) * fc);
    memcpy(d->player_vertices, verts, sizeof(Vec3) * vc);
    memcpy(d->player_faces, faces, sizeof(Face) * fc);
    d->player_vcount = vc;
    d->player_fcount = fc;

    make_plane(
        &d->ground_vertices,
        &d->ground_faces,
        &d->ground_vcount,
        &d->ground_fcount,
        10, 10, 0
    );

    s->data = d;
    s->vtable = &vtable;
    return s;
}
