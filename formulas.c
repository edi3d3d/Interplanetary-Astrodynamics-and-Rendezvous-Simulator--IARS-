#include "formulas.h"

Vec3 gravitationalAcceleration(float mass, Vec3 relativeVector) {
    const float G = 6.67430e-11f;

    float distanceSquared = v3_dot(relativeVector, relativeVector);
    float invDistance = 1.0f / sqrtf(distanceSquared);
    float invDistance3 = invDistance * invDistance * invDistance;

    return v3_scale(relativeVector, G * mass * invDistance3);
}