#include "scene.h"
#include <stdlib.h>

static Scene* current_scene = NULL;

void scene_manager_set(Scene* scene) {
    if (current_scene && current_scene->vtable && current_scene->vtable->destroy) {
        current_scene->vtable->destroy(current_scene);
        free(current_scene);
    }
    current_scene = scene;
    if (current_scene && current_scene->vtable && current_scene->vtable->init) {
        current_scene->vtable->init(current_scene);
    }
}

void scene_manager_update(float delta_time, Input* input, Camera* camera, Mat4 proj) {
    if (current_scene && current_scene->vtable && current_scene->vtable->update) {
        current_scene->vtable->update(current_scene, delta_time, input, camera, proj);
    }
}

void scene_manager_render(Renderer* renderer) {
    if (current_scene && current_scene->vtable && current_scene->vtable->render) {
        current_scene->vtable->render(current_scene, renderer);
    }
}

void scene_manager_destroy() {
    if (current_scene && current_scene->vtable && current_scene->vtable->destroy) {
        current_scene->vtable->destroy(current_scene);
        free(current_scene);
    }
    current_scene = NULL;
}
