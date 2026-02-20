#include "vec3.h"
#include <string.h>

void v3_set(Vec3 *out, float x, float y, float z) {
    if (!out) return;
    out->x = x; out->y = y; out->z = z;
}

void v3_copy(Vec3 *out, const Vec3 *a) {
    if (!out || !a) return;
    memcpy(out, a, sizeof(Vec3));
}

void v3_add(Vec3 *out, const Vec3 *a, const Vec3 *b) {
    if (!out || !a || !b) return;
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
}

void v3_sub(Vec3 *out, const Vec3 *a, const Vec3 *b) {
    if (!out || !a || !b) return;
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
}

void v3_scale(Vec3 *out, const Vec3 *a, float s) {
    if (!out || !a) return;
    out->x = a->x * s;
    out->y = a->y * s;
    out->z = a->z * s;
}

void v3_mul(Vec3 *out, const Vec3 *a, const Vec3 *b) {
    if (!out || !a || !b) return;
    out->x = a->x * b->x;
    out->y = a->y * b->y;
    out->z = a->z * b->z;
}

float v3_dot(const Vec3 *a, const Vec3 *b) {
    if (!a || !b) return 0.0f;
    return a->x*b->x + a->y*b->y + a->z*b->z;
}

void v3_cross(Vec3 *out, const Vec3 *a, const Vec3 *b) {
    if (!out || !a || !b) return;
    out->x = a->y*b->z - a->z*b->y;
    out->y = a->z*b->x - a->x*b->z;
    out->z = a->x*b->y - a->y*b->x;
}

float v3_len2(const Vec3 *a) {
    if (!a) return 0.0f;
    return a->x*a->x + a->y*a->y + a->z*a->z;
}

float v3_len(const Vec3 *a) {
    return sqrtf(v3_len2(a));
}

void v3_normalize(Vec3 *out, const Vec3 *a) {
    if (!out || !a) return;
    float l = v3_len(a);
    if (l > 1e-6f) {
        out->x = a->x / l;
        out->y = a->y / l;
        out->z = a->z / l;
    } else {
        out->x = out->y = out->z = 0.0f;
    }
}

void v3_add_scaled(Vec3 *out, const Vec3 *a, const Vec3 *b, float s) {
    if (!out || !a || !b) return;
    out->x = a->x + b->x * s;
    out->y = a->y + b->y * s;
    out->z = a->z + b->z * s;
}

void v3_scale_inplace(Vec3 *a, float s) {
    if (!a) return;
    a->x *= s; a->y *= s; a->z *= s;
}
