#include "scene_factory.h"
#include "teapot_scene.h"
#include "game_scene.h"

Scene* scene_factory_create_start_scene(const Vec3* vertices, const Face* faces, size_t vcount, size_t fcount, int width, int height) {
    return teapot_scene_create(vertices, faces, vcount, fcount, width, height);
}

Scene* scene_factory_create_game_scene(const Vec3* vertices, const Face* faces, size_t vcount, size_t fcount, int width, int height) {
    return game_scene_create(vertices, faces, vcount, fcount, width, height);
}
