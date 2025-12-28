#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "core/mat.h"
#include "scene/teapot_renderer.h"

typedef struct GameObject GameObject;

typedef enum {
    GO_TYPE_MESH = 0,
} GameObjectType;

struct GameObject {
    GameObjectType type;
    Mat4 model;
    TeapotRenderer* mesh;
    int visible;
};

GameObject* game_object_create_mesh(TeapotRenderer* mesh, Mat4 model);
void game_object_destroy(GameObject* go);

#endif // GAME_OBJECT_H
