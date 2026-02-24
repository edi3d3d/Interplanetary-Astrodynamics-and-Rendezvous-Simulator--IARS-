#include "vec3.h"
#include <string.h>
#include <math.h>

Vec3 v3_set(float x, float y, float z) {
    return (Vec3){x, y, z};
}

Vec3 v3_copy(const Vec3 a) {
    return v3_set(a.x, a.y, a.z);
}

Vec3 v3_add(const Vec3 a, const Vec3 b) {
    return v3_set(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 v3_sub(const Vec3 a, const Vec3 b) {
    return v3_set(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 v3_scale(const Vec3 a, float s) {
    return v3_set(a.x * s, a.y * s, a.z * s);
}

Vec3 v3_mul(const Vec3 a, const Vec3 b) {
    return v3_set(a.x * b.x, a.y * b.y, a.z * b.z);
}

float v3_dot(const Vec3 a, const Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 v3_cross(const Vec3 a, const Vec3 b) {
    return v3_set(a.y*b.z - a.z*b.y,
                  a.z*b.x - a.x*b.z,
                  a.x*b.y - a.y*b.x);
}

float v3_len2(const Vec3 a) {
    return a.x*a.x + a.y*a.y + a.z*a.z;
}

float v3_len(const Vec3 a) {
    return sqrtf(v3_len2(a));
}

int v3_is_zero(const Vec3 a) {
    return v3_len2(a) < 1e-6f;
}

Vec3 v3_normalize(const Vec3 a) {
    float l = v3_len(a);
    if (l > 1e-6f) return v3_scale(a, 1.0f / l);
    return v3_set(0,0,0);
}

Vec3 v3_normalize_to(const Vec3 a, const float s) {
    float l = v3_len(a);
    if (l > 1e-6f) return v3_scale(a, s / l);
    return v3_set(0,0,0);
}

Vec3 v3_add_scaled(const Vec3 a, const Vec3 b, float s) {
    return v3_add(a, v3_scale(b, s));
}

void v3_scale_inplace(Vec3 *a, float s) {
    if (!a) return;
    *a = v3_scale(*a, s);
}
