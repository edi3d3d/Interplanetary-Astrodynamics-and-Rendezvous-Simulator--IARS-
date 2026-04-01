#ifndef PLANET_H
#define PLANET_H

#include "vec3.h"

typedef struct Planet {
    Vec3 position;   // km, float precision
    Vec3 velocity;   // km/s, float precision
    float mass;      // kg
    float radius;    // km
} Planet;

Planet create_planet(Vec3 position, Vec3 velocity, float mass, float radius);
void change_planet_position(Planet *planet, Vec3 dt, int add);
void update_planet_position(Planet *planet, Vec3 dt);
void planet_gravity_update(Planet *planet, Planet *other, float dt);

#endif