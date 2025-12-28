#include "teapot_scene.h"
#include "../scene/teapot_renderer.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    TeapotRenderer* teapot;
    Vec3* vertices;
    Face* faces;
    size_t vertex_count;
    size_t face_count;
    int wireframe;
    float angle;
    Mat4 model, view, proj;
    Vec3 camera_pos;
    int width, height;
} TeapotSceneData;

static void teapot_scene_init(Scene* scene) {
    TeapotSceneData* data = (TeapotSceneData*)scene->data;
    data->teapot = teapot_renderer_create(data->vertices, data->faces, data->vertex_count, data->face_count);
}

static void teapot_scene_update(Scene* scene, float delta_time, Input* input, Camera* camera, Mat4 proj) {
    TeapotSceneData* data = (TeapotSceneData*)scene->data;
    if (input->keyboard.pressed[SDL_SCANCODE_TAB]) {
        data->wireframe = !data->wireframe;
    }

    data->angle += delta_time * 0.5f;
    if (data->angle > 2.0f * 3.14159265f) data->angle -= 2.0f * 3.14159265f;

    Mat4 rotation = mat4_rotation_y(data->angle);
    Mat4 translation = mat4_translation((Vec3){0,0,-5});
    Mat4 model = mat4_mul(translation, rotation);

    data->proj = proj;
    data->view = camera_get_view(camera);
    data->camera_pos = camera->position;

    teapot_renderer_update(data->teapot, model, data->view, data->proj, data->camera_pos, data->width, data->height);
}

static void teapot_scene_render(Scene* scene, Renderer* renderer) {
    TeapotSceneData* data = (TeapotSceneData*)scene->data;
    teapot_renderer_draw(data->teapot, renderer, data->wireframe);
}

static void teapot_scene_destroy(Scene* scene) {
    TeapotSceneData* data = (TeapotSceneData*)scene->data;
    if (data->teapot) teapot_renderer_destroy(data->teapot);
    free(data->vertices);
    free(data->faces);
    free(data);
    scene->data = NULL;
}

static SceneVTable teapot_scene_vtable = {
    .init = teapot_scene_init,
    .update = teapot_scene_update,
    .render = teapot_scene_render,
    .destroy = teapot_scene_destroy
};

Scene* teapot_scene_create(const Vec3* vertices, const Face* faces, size_t vertex_count, size_t face_count, int width, int height) {
    Scene* scene = malloc(sizeof(Scene));
    TeapotSceneData* data = malloc(sizeof(TeapotSceneData));
    data->vertices = malloc(sizeof(Vec3) * vertex_count);
    memcpy(data->vertices, vertices, sizeof(Vec3) * vertex_count);
    data->faces = malloc(sizeof(Face) * face_count);
    memcpy(data->faces, faces, sizeof(Face) * face_count);
    data->vertex_count = vertex_count;
    data->face_count = face_count;
    data->wireframe = 0;
    data->angle = 0.0f;
    data->proj = mat4_identity();
    data->view = mat4_identity();
    data->camera_pos = (Vec3){0,0,0};
    data->width = width;
    data->height = height;
    scene->data = data;
    scene->vtable = &teapot_scene_vtable;
    return scene;
}
