#include "teapot_renderer.h"
#include "core/geom.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "core/culling.h"
#include "core/math.h"

struct TeapotRenderer {
    const Vec3* vertices;
    const Face* faces;
    size_t vertex_count;
    size_t face_count;

    Vec3* view_space_positions;
    Vec3* projected_positions;
    unsigned char* vertex_valid;
    Vec3* vertex_normals;
    uint32_t* vertex_colors;

    Vec3 center;
    float radius;
    int last_visible;
    int last_inside;
};

static const float NEAR_PLANE = 0.1f;
static const float MIN_AREA_INSIDE = 4.0f;
static const float MIN_AREA_OUTSIDE = 8.0f;
static const size_t MAX_PRIMITIVES = 20000;

TeapotRenderer* teapot_renderer_create(const Vec3* vertices, const Face* faces, size_t vertex_count, size_t face_count) {
    TeapotRenderer* t = malloc(sizeof(*t));
    if (!t) return NULL;
    memset(t, 0, sizeof(*t));

    t->vertices = vertices;
    t->faces = faces;
    t->vertex_count = vertex_count;
    t->face_count = face_count;

    if (vertex_count > 0) {
        t->view_space_positions = calloc(vertex_count, sizeof(Vec3));
        t->projected_positions  = calloc(vertex_count, sizeof(Vec3));
        t->vertex_valid         = calloc(vertex_count, sizeof(unsigned char));
        t->vertex_normals       = calloc(vertex_count, sizeof(Vec3));
        t->vertex_colors        = calloc(vertex_count, sizeof(uint32_t));
    }

    // compute center and bounding radius
    if (vertex_count > 0) {
        Vec3 sum = {0,0,0};
        for (size_t i = 0; i < vertex_count; ++i) sum = vec3_add(sum, vertices[i]);
        t->center = vec3_scale(sum, 1.0f / (float)vertex_count);
        float maxd = 0.0f;
        for (size_t i = 0; i < vertex_count; ++i) {
            float d = vec3_length(vec3_sub(vertices[i], t->center));
            if (d > maxd) maxd = d;
        }
        t->radius = maxd;
    } else {
        t->center = (Vec3){0,0,0};
        t->radius = 1.0f;
    }

    t->last_visible = 1;

    // compute per-vertex normals by averaging face normals
    for (size_t i = 0; i < vertex_count; ++i) t->vertex_normals[i] = (Vec3){0,0,0};
    for (size_t i = 0; i < face_count; ++i) {
        Face f = faces[i];
        if ((size_t)f.v1 >= vertex_count || (size_t)f.v2 >= vertex_count || (size_t)f.v3 >= vertex_count) continue;
        Vec3 a = vertices[f.v1];
        Vec3 b = vertices[f.v2];
        Vec3 c = vertices[f.v3];
        Vec3 fn = vec3_normalize(vec3_cross(vec3_sub(b,a), vec3_sub(c,a)));
        t->vertex_normals[f.v1] = vec3_add(t->vertex_normals[f.v1], fn);
        t->vertex_normals[f.v2] = vec3_add(t->vertex_normals[f.v2], fn);
        t->vertex_normals[f.v3] = vec3_add(t->vertex_normals[f.v3], fn);
    }
    for (size_t i = 0; i < vertex_count; ++i) t->vertex_normals[i] = vec3_normalize(t->vertex_normals[i]);

    return t;
}

void teapot_renderer_destroy(TeapotRenderer* t) {
    if (!t) return;
    free(t->view_space_positions);
    free(t->projected_positions);
    free(t->vertex_valid);
    free(t->vertex_normals);
    free(t->vertex_colors);
    free(t);
}

int teapot_renderer_update(TeapotRenderer* t, Mat4 model, Mat4 view, Mat4 proj, Vec3 camera_pos, int width, int height) {
    if (!t) return 0;

    Vec3 world_center = geom_transform_point(model, t->center);
    if (!sphere_in_frustum(view, world_center, t->radius, width, height, 3.14159265f/3.0f, NEAR_PLANE, 100.0f)) {
        t->last_visible = 0;
        return 0;
    }

    Mat4 view_model = mat4_mul(view, model);
    // light direction: from above and at an angle
    Vec3 light_pos = vec3_add(world_center, (Vec3){2.0f, 5.0f, 3.0f});
    Vec3 light = vec3_normalize(vec3_sub(light_pos, world_center));
    const float ambient = 0.15f;

    for (size_t i = 0; i < t->vertex_count; ++i) {
        Vec3 world = geom_transform_point(model, t->vertices[i]);
        Vec3 screen, view_space;
        if (!geom_project_point(view, proj, world, width, height, &screen, &view_space)) {
            t->vertex_valid[i] = 0;
            t->projected_positions[i] = (Vec3){INFINITY,INFINITY,INFINITY};
            continue;
        }
        t->view_space_positions[i] = view_space;
        t->projected_positions[i] = screen;
        t->vertex_valid[i] = 1;

        Vec3 n = mat4_mul_vec3_dir(view_model, t->vertex_normals[i]);
        n = vec3_normalize(n);
        float diff = fmaxf(vec3_dot(n,light), 0.0f);
        float intensity = ambient + diff*(1.0f - ambient);
        uint8_t c = (uint8_t)(clampf(intensity*255.0f,0.0f,255.0f));
        t->vertex_colors[i] = 0xFF000000 | (c<<16) | (c<<8) | c;
    }

    t->last_inside = (vec3_length(vec3_sub(camera_pos, world_center)) < t->radius);
    t->last_visible = 1;
    return 1;
}

void teapot_renderer_draw(TeapotRenderer* t, Renderer* r, int wireframe_pref) {
    size_t primitives_drawn = 0;
    for (size_t i = 0; i < t->face_count && primitives_drawn < MAX_PRIMITIVES; ++i) {
        Face f = t->faces[i];
        int idxs[3] = {f.v1,f.v2,f.v3};
        if ((size_t)idxs[0] >= t->vertex_count || (size_t)idxs[1] >= t->vertex_count || (size_t)idxs[2] >= t->vertex_count) continue;
        if (!t->vertex_valid[idxs[0]] || !t->vertex_valid[idxs[1]] || !t->vertex_valid[idxs[2]]) continue;

        Vec3 s0 = t->projected_positions[idxs[0]];
        Vec3 s1 = t->projected_positions[idxs[1]];
        Vec3 s2 = t->projected_positions[idxs[2]];

        float area = fabsf((s1.x-s0.x)*(s2.y-s0.y)-(s1.y-s0.y)*(s2.x-s0.x));
        if (!t->last_inside && geom_triangle_backface_cull((Vec3[]){s0,s1,s2})) continue;
        if (t->last_inside && area < MIN_AREA_INSIDE) continue;
        if (!t->last_inside && area < MIN_AREA_OUTSIDE) continue;

        float vz0 = t->view_space_positions[idxs[0]].z;
        float vz1 = t->view_space_positions[idxs[1]].z;
        float vz2 = t->view_space_positions[idxs[2]].z;
        if (vz0 > -NEAR_PLANE || vz1 > -NEAR_PLANE || vz2 > -NEAR_PLANE) continue;

        if (wireframe_pref) {
            renderer_draw_line(r,s0,s1,0xFFFFFFFF);
            renderer_draw_line(r,s1,s2,0xFFFFFFFF);
            renderer_draw_line(r,s2,s0,0xFFFFFFFF);
            primitives_drawn += 3;
        } else {
            renderer_draw_triangle_shaded(r, s0,s1,s2,
                                         t->vertex_colors[idxs[0]],
                                         t->vertex_colors[idxs[1]],
                                         t->vertex_colors[idxs[2]]);
            primitives_drawn += 1;
        }
    }
}
