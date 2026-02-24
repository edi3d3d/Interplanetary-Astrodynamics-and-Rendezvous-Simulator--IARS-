#ifndef QUATERNIONS_H
#define QUATERNIONS_H
#include "vec3.h"

typedef struct Quaternion{
     float w;
     Vec3 v;
} Quaternion;

Quaternion q_set(float w, float x, float y, float z);
Quaternion q_set_vec(float w, const Vec3 v);
Quaternion q_copy(const Quaternion q);
Quaternion q_conjugate(const Quaternion q);
Quaternion q_mul(const Quaternion a, const Quaternion b);
Quaternion q_mul_vec(const Quaternion q, const Vec3 v);
Quaternion q_normalize(const Quaternion q);
Quaternion q_from_axis_angle(Vec3 axis_unit, float angleRad);
Vec3 q_rotate_vec3(Quaternion q, Vec3 v);

#endif // QUATERNIONS_H