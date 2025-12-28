#ifndef CORE_CULLING_H
#define CORE_CULLING_H

#include "mat.h"
#include "vec.h"

int sphere_in_frustum(Mat4 view, Vec3 world_center, float radius, int width, int height, float fov, float near_plane, float far_plane);

#endif // CORE_CULLING_H
