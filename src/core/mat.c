#include "mat.h"
#include <string.h>

Mat4 mat4_identity(void) {
    Mat4 r;
    memset(&r, 0, sizeof(r));
    r.m[0][0] = 1.0f;
    r.m[1][1] = 1.0f;
    r.m[2][2] = 1.0f;
    r.m[3][3] = 1.0f;
    return r;
}

Mat4 mat4_mul(Mat4 a, Mat4 b) {
    Mat4 r;
    memset(&r, 0, sizeof(r));

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                r.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return r;
}

Vec4 mat4_mul_vec4(Mat4 m, Vec4 v) {
    Vec4 r;
    r.x = m.m[0][0]*v.x + m.m[0][1]*v.y + m.m[0][2]*v.z + m.m[0][3]*v.w;
    r.y = m.m[1][0]*v.x + m.m[1][1]*v.y + m.m[1][2]*v.z + m.m[1][3]*v.w;
    r.z = m.m[2][0]*v.x + m.m[2][1]*v.y + m.m[2][2]*v.z + m.m[2][3]*v.w;
    r.w = m.m[3][0]*v.x + m.m[3][1]*v.y + m.m[3][2]*v.z + m.m[3][3]*v.w;
    return r;
}


Mat4 mat4_translation(Vec3 t) {
    Mat4 r = mat4_identity();
    r.m[0][3] = t.x;
    r.m[1][3] = t.y;
    r.m[2][3] = t.z;
    return r;
}

Mat4 mat4_scale(Vec3 s) {
    Mat4 r = mat4_identity();
    r.m[0][0] = s.x;
    r.m[1][1] = s.y;
    r.m[2][2] = s.z;
    return r;
}

Mat4 mat4_rotation_x(float radians) {
    Mat4 r = mat4_identity();
    float c = cosf(radians);
    float s = sinf(radians);
    r.m[1][1] = c; r.m[1][2] = -s;
    r.m[2][1] = s; r.m[2][2] = c;
    return r;
}

Mat4 mat4_rotation_y(float radians) {
    Mat4 r = mat4_identity();
    float c = cosf(radians);
    float s = sinf(radians);
    r.m[0][0] = c; r.m[0][2] = s;
    r.m[2][0] = -s; r.m[2][2] = c;
    return r;
}

Mat4 mat4_rotation_z(float radians) {
    Mat4 r = mat4_identity();
    float c = cosf(radians);
    float s = sinf(radians);
    r.m[0][0] = c; r.m[0][1] = -s;
    r.m[1][0] = s; r.m[1][1] = c;
    return r;
}

Mat4 mat4_perspective(float fov, float aspect, float near, float far) {
    float f = 1.0f / tanf(fov / 2.0f);
    Mat4 m;
    memset(&m, 0, sizeof(m));
    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = (far + near) / (near - far);
    m.m[2][3] = (2.0f * far * near) / (near - far);
    m.m[3][2] = -1.0f;
    return m;
}

Vec3 mat4_mul_vec3(Mat4 m, Vec3 v) {
    Vec4 r4 = mat4_mul_vec4(m, (Vec4){ v.x, v.y, v.z, 1.0f });
    if (fabsf(r4.w) > 1e-9f) {
        return (Vec3){ r4.x / r4.w, r4.y / r4.w, r4.z / r4.w };
    }
    return (Vec3){ r4.x, r4.y, r4.z };
}
