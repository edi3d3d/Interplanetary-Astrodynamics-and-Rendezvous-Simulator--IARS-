#include "quaternions.h"
#include <math.h>

Quaternion q_set(float w, float x, float y, float z) {
    return (Quaternion){w, v3_set(x,y,z)};
}

Quaternion q_set_vec(float w, const Vec3 v) {
    return q_set(w, v.x, v.y, v.z);
}

Quaternion q_copy(const Quaternion q) {
    return q_set(q.w, q.v.x, q.v.y, q.v.z);
}

Quaternion q_conjugate(const Quaternion q) {
    return q_set(q.w, -q.v.x, -q.v.y, -q.v.z);
}

Quaternion q_mul(const Quaternion a, const Quaternion b){
    return q_set(
        a.w*b.w   - a.v.x*b.v.x - a.v.y*b.v.y - a.v.z*b.v.z,
        a.w*b.v.x + a.v.x*b.w   + a.v.y*b.v.z - a.v.z*b.v.y,
        a.w*b.v.y - a.v.x*b.v.z + a.v.y*b.w   + a.v.z*b.v.x,
        a.w*b.v.z + a.v.x*b.v.y - a.v.y*b.v.x + a.v.z*b.w
    );
}

Quaternion q_normalize(const Quaternion q){
    float len = sqrtf(q.w*q.w + v3_len2(q.v));
    if (len > 1e-6f) {
        float invLen = 1.0f / len;
        return q_set_vec(q.w * invLen, v3_scale(q.v, invLen));
    }
    return q_set(1,0,0,0); // default to identity quaternion if zero-length
}

Quaternion q_from_axis_angle(Vec3 axis_unit, float angleRad)
{
    float h = 0.5f * angleRad;
    float s = sinf(h);
    return q_normalize(q_set(cosf(h), axis_unit.x*s, axis_unit.y*s, axis_unit.z*s));
}

Vec3 q_rotate_vec3(Quaternion q, Vec3 v)
{
    q = q_normalize(q);
    Quaternion p = q_set(0, v.x, v.y, v.z);
    Quaternion qc = q_conjugate(q);
    Quaternion r = q_mul(q_mul(q, p), qc);
    return v3_set(r.v.x, r.v.y, r.v.z);
}