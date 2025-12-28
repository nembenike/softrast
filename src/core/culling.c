#include "culling.h"
#include <math.h>

int sphere_in_frustum(Mat4 view, Vec3 world_center, float radius, int width, int height, float fov, float near_plane, float far_plane) {
    Vec3 view_center = mat4_mul_vec3(view, world_center);
    float view_depth = -view_center.z; // positive in front
    float tan_half = tanf(fov * 0.5f);
    float aspect = (float)width / (float)height;
    float x_limit = view_depth * tan_half * aspect;
    float y_limit = view_depth * tan_half;

    if ((view_depth - radius) > far_plane) return 0;
    if ((view_depth + radius) < near_plane) return 0;
    if (view_depth > 0.0f) {
        if ((fabsf(view_center.x) - radius) > x_limit) return 0;
        if ((fabsf(view_center.y) - radius) > y_limit) return 0;
    }
    return 1;
}
