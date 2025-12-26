#ifndef MAT_H
#define MAT_H

#include "vec.h"

// Row-major 4x4 matrix
// Multiplied as: result = M * v (column vector)
typedef struct {
    float m[4][4];
} Mat4;

Mat4 mat4_identity(void);
Mat4 mat4_mul(Mat4 a, Mat4 b);
Vec4 mat4_mul_vec4(Mat4 m, Vec4 v);

// Common constructors
Mat4 mat4_translation(Vec3 t);
Mat4 mat4_scale(Vec3 s);
Mat4 mat4_rotation_x(float radians);
Mat4 mat4_rotation_y(float radians);
Mat4 mat4_rotation_z(float radians);
Mat4 mat4_perspective(float fov, float aspect, float near, float far);

#endif // MAT_H
