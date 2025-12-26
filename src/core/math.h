#ifndef MATH_H
#define MATH_H

static inline float clampf(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

static inline float lerpf(float a, float b, float t) {
    return a + t * (b - a);
}

// Interpolates a value using barycentric coordinates (u, v)
// Assumes w = 1 - u - v
static inline float barycentric_interp(float v0, float v1, float v2, float u, float v) {
    return (1.0f - u - v) * v0 + u * v1 + v * v2;
}

#endif // MATH_H
