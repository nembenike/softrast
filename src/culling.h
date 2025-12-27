#ifndef CULLING_H
#define CULLING_H

#include "core/mat.h"
#include "core/vec.h"

int sphere_in_frustum(Mat4 view, Vec3 world_center, float radius, int width, int height, float fov, float near_plane, float far_plane);

#endif // CULLING_H
