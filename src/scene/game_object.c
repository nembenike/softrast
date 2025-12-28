#include "game_object.h"
#include <stdlib.h>

GameObject* game_object_create_mesh(TeapotRenderer* mesh, Mat4 model) {
    GameObject* go = malloc(sizeof(GameObject));
    if (!go) return NULL;
    go->type = GO_TYPE_MESH;
    go->model = model;
    go->mesh = mesh;
    go->visible = 1;
    return go;
}

void game_object_destroy(GameObject* go) {
    if (!go) return;
    free(go);
}
