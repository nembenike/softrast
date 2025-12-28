#ifndef SCENE_H
#define SCENE_H

#include "../renderer/renderer.h"
#include "../platform/input.h"
#include "../core/camera.h"
#include "../core/mat.h"

typedef struct Scene Scene;

typedef struct SceneVTable {
    void (*init)(Scene* scene);
    void (*update)(Scene* scene, float delta_time, Input* input, Camera* camera, Mat4 proj);
    void (*render)(Scene* scene, Renderer* renderer);
    void (*destroy)(Scene* scene);
} SceneVTable;

struct Scene {
    void* data;
    SceneVTable* vtable;
};

// Scene manager API
void scene_manager_set(Scene* scene);
void scene_manager_update(float delta_time, Input* input, Camera* camera, Mat4 proj);
void scene_manager_render(Renderer* renderer);
void scene_manager_destroy();

#endif // SCENE_H
