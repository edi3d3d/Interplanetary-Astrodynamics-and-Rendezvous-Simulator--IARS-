#include "formulas.h"

Vec3 gravitationalAcceleration(DATA mass, Vec3 relativeVector) {
    const DATA G = 6.67430e-11;

    DATA distanceSquared = v3_dot(relativeVector, relativeVector);
    if (distanceSquared <= 0) return v3_set(0,0,0);
    DATA invDistance = (DATA)1.0 / sqrt(distanceSquared);
    DATA invDistance3 = invDistance * invDistance * invDistance;

    return v3_scale(relativeVector, G * mass * invDistance3);
}
