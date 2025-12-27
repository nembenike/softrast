#include "geom.h"
#include <math.h>
#include <float.h>

extern Vec3 ndc_to_screen(Vec3 v, int width, int height); // declared in renderer.h but defined there

int geom_project_point(Mat4 view, Mat4 proj, Vec3 world, int width, int height, Vec3* out_screen, Vec3* out_view_space) {
    Vec3 view_space = mat4_mul_vec3(view, world);
    if (out_view_space) *out_view_space = view_space;
    Vec4 clip = mat4_mul_vec4(proj, (Vec4){view_space.x, view_space.y, view_space.z, 1.0f});
    if (fabsf(clip.w) < 1e-6f) return 0;
    Vec3 ndc = { clip.x/clip.w, clip.y/clip.w, clip.z/clip.w };
    ndc.z = (ndc.z+1.0f)*0.5f;
    if (out_screen) *out_screen = ndc_to_screen(ndc, width, height);
    return 1;
}

int geom_triangle_backface_cull(const Vec3 screen[3]) {
    float area = (screen[1].x - screen[0].x) * (screen[2].y - screen[0].y)
               - (screen[1].y - screen[0].y) * (screen[2].x - screen[0].x);
    if (fabsf(area) < 1e-6f) return 1; // degenerate
    if (area < 0.0f) {
        // flip winding
        Vec3 tmp = screen[1];
        ((Vec3*)screen)[1] = screen[2];
        ((Vec3*)screen)[2] = tmp;
    }
    return 0;
}

Vec3 mat4_mul_vec3_dir(Mat4 m, Vec3 v) {
    Vec3 r;
    r.x = m.m[0][0]*v.x + m.m[0][1]*v.y + m.m[0][2]*v.z;
    r.y = m.m[1][0]*v.x + m.m[1][1]*v.y + m.m[1][2]*v.z;
    r.z = m.m[2][0]*v.x + m.m[2][1]*v.y + m.m[2][2]*v.z;
    return r;
}
