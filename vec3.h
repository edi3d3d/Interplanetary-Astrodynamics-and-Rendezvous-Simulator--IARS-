#ifndef VEC3_H
#define VEC3_H

#include <math.h>

typedef struct Vec3{
    float x;
    float y;
    float z;
} Vec3;

// Construct and return vector
Vec3 v3_set(float x, float y, float z);
// Return a copy
Vec3 v3_copy(const Vec3 a);
// out = a + b
Vec3 v3_add(const Vec3 a, const Vec3 b);
// out = a - b
Vec3 v3_sub(const Vec3 a, const Vec3 b);
// out = a * s
Vec3 v3_scale(const Vec3 a, float s);
// out = a * b (component-wise)
Vec3 v3_mul(const Vec3 a, const Vec3 b);
// dot product
float v3_dot(const Vec3 a, const Vec3 b);
// cross product out = a x b (returns vector)
Vec3 v3_cross(const Vec3 a, const Vec3 b);
// squared length
float v3_len2(const Vec3 a);
// length
float v3_len(const Vec3 a);
// is a zero vector (length == 0)
int v3_is_zero(const Vec3 a);
// return normalize(a) (if zero-length, returns 0)
Vec3 v3_normalize(const Vec3 a);
// return vector a normalized to length s (if zero-length, returns 0)
Vec3 v3_normalize_to(const Vec3 a, const float s);
// return a + s * b
Vec3 v3_add_scaled(const Vec3 a, const Vec3 b, float s);
// in-place scale a *= s
void v3_scale_inplace(Vec3 *a, float s);

#endif // VEC3_H
