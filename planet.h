#ifndef PLANET_H
#define PLANET_H

#include "vec3.h"
#include "formulas.h"

typedef struct Planet {
    Vec3 position;     // km, float precision
    Vec3 velocity;     // km/s, float precision
    Vec3 acceleration; // km/s^2, float precision
    float mass;        // kg
    float radius;      // km
} Planet;

Planet create_planet(Vec3 position, Vec3 velocity, float mass, float radius);
void change_planet_position(Planet *planet, Vec3 dt, int add);
void planetGravityUpdate(Planet *bodies, int bodyCount, float dt);
double systemEnergy(const Planet *bodies, int bodyCount);

#endif