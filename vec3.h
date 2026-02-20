#ifndef VEC3_H
#define VEC3_H

#include <math.h>

typedef struct Vec3 {
    float x;
    float y;
    float z;
} Vec3;

// Set components
void v3_set(Vec3 *out, float x, float y, float z);
// Copy
void v3_copy(Vec3 *out, const Vec3 *a);
// out = a + b
void v3_add(Vec3 *out, const Vec3 *a, const Vec3 *b);
// out = a - b
void v3_sub(Vec3 *out, const Vec3 *a, const Vec3 *b);
// out = a * s
void v3_scale(Vec3 *out, const Vec3 *a, float s);
// out = a * b (component-wise)
void v3_mul(Vec3 *out, const Vec3 *a, const Vec3 *b);
// dot product
float v3_dot(const Vec3 *a, const Vec3 *b);
// cross product out = a x b
void v3_cross(Vec3 *out, const Vec3 *a, const Vec3 *b);
// squared length
float v3_len2(const Vec3 *a);
// length
float v3_len(const Vec3 *a);
// out = normalize(a) (if zero-length, out set to 0)
void v3_normalize(Vec3 *out, const Vec3 *a);
// out = a + s * b
void v3_add_scaled(Vec3 *out, const Vec3 *a, const Vec3 *b, float s);
// in-place scale a *= s
void v3_scale_inplace(Vec3 *a, float s);

#endif // VEC3_H
