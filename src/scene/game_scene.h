#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include "scene/scene.h"
#include "core/vec.h"
#include "assets/objloader.h"

Scene* game_scene_create(const Vec3* vertices, const Face* faces, size_t vertex_count, size_t face_count, int width, int height);

#endif // GAME_SCENE_H
