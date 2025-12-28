#include "model.h"
#include <float.h>
#include "../core/vec.h"

void normalize_model(Vec3* vertices, size_t vertex_count, float target_size) {
    if (!vertices || vertex_count == 0) return;

    Vec3 min = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
    Vec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (size_t i = 0; i < vertex_count; i++) {
        Vec3 v = vertices[i];
        if (v.x < min.x) min.x = v.x;
        if (v.y < min.y) min.y = v.y;
        if (v.z < min.z) min.z = v.z;
        if (v.x > max.x) max.x = v.x;
        if (v.y > max.y) max.y = v.y;
        if (v.z > max.z) max.z = v.z;
    }

    Vec3 center = {
        (min.x + max.x) * 0.5f,
        (min.y + max.y) * 0.5f,
        (min.z + max.z) * 0.5f
    };

    float dx = max.x - min.x;
    float dy = max.y - min.y;
    float dz = max.z - min.z;

    float max_dim = fmaxf(dx, fmaxf(dy, dz));
    if (max_dim <= 0.00001f) return;

    float scale = target_size / max_dim;

    for (size_t i = 0; i < vertex_count; i++) {
        vertices[i].x = (vertices[i].x - center.x) * scale;
        vertices[i].y = (vertices[i].y - center.y) * scale;
        vertices[i].z = (vertices[i].z - center.z) * scale;
    }
}

