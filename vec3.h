#ifndef VEC3_H
#define VEC3_H

#include <math.h>
#include <stdio.h>
#include "units.h"

typedef struct Vec3{
    DATA x;
    DATA y;
    DATA z;
} Vec3;

// Construct and return vector
Vec3 v3_set(DATA x, DATA y, DATA z);
// Return a copy
Vec3 v3_copy(const Vec3 a);
// out = a + b
Vec3 v3_add(const Vec3 a, const Vec3 b);
// out = a - b
Vec3 v3_sub(const Vec3 a, const Vec3 b);
// out = a * s
Vec3 v3_scale(const Vec3 a, DATA s);
// out = a * b (component-wise)
Vec3 v3_mul(const Vec3 a, const Vec3 b);
// dot product
DATA v3_dot(const Vec3 a, const Vec3 b);
// cross product out = a x b (returns vector)
Vec3 v3_cross(const Vec3 a, const Vec3 b);
// squared length
DATA v3_len2(const Vec3 a);
// length
DATA v3_len(const Vec3 a);
// is a zero vector (length == 0)
int v3_is_zero(const Vec3 a);
// return normalize(a) (if zero-length, returns 0)
Vec3 v3_normalize(const Vec3 a);
// return vector a normalized to length s (if zero-length, returns 0)
Vec3 v3_normalize_to(const Vec3 a, const DATA s);
// return a + s * b
Vec3 v3_add_scaled(const Vec3 a, const Vec3 b, DATA s);
// in-place scale a *= s
void v3_scale_inplace(Vec3 *a, DATA s);
// print vector to console (for debugging)
void v3_print(const Vec3 a);
#endif // VEC3_H
