#ifndef CORE_GEOM_H
#define CORE_GEOM_H

#include "mat.h"
#include "vec.h"

static inline Vec3 geom_transform_point(Mat4 m, Vec3 v) {
    return mat4_mul_vec3(m, v);
}

int geom_project_point(Mat4 view, Mat4 proj, Vec3 world, int width, int height, Vec3* out_screen, Vec3* out_view_space);

int geom_triangle_backface_cull(const Vec3 screen[3]);

Vec3 mat4_mul_vec3_dir(Mat4 m, Vec3 v);

#endif // CORE_GEOM_H
