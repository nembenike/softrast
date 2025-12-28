#ifndef SCENE_FACTORY_H
#define SCENE_FACTORY_H

#include "scene.h"
#include "../core/vec.h"
#include "../assets/objloader.h"

Scene* scene_factory_create_start_scene(const Vec3* vertices, const Face* faces, size_t vcount, size_t fcount, int width, int height);

#endif // SCENE_FACTORY_H
