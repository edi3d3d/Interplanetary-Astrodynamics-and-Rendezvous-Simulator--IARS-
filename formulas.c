#include "formulas.h"

Vec3 gravitationalAcceleration(DATA mass, Vec3 relativeVector) {
    const DATA G = 6.67430e-11f;

    DATA distanceSquared = v3_dot(relativeVector, relativeVector);
    DATA invDistance = 1.0f / sqrtl(distanceSquared);
    DATA invDistance3 = invDistance * invDistance * invDistance;

    return v3_scale(relativeVector, G * mass * invDistance3);
}