#ifndef PLANET_H
#define PLANET_H

#include "vec3.h"

typedef struct Planet {
    Vec3 position;
    Vec3 velocity;
    float mass;
    float radius;
} Planet;

Planet create_planet(Vec3 position, Vec3 velocity, float mass, float radius);
#endif