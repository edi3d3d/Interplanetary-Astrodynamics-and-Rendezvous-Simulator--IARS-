#ifndef PLANET_H
#define PLANET_H

#include "vec3.h"
#include "formulas.h"
#include "units.h"

#define MAX_BODIES 16

typedef struct GravityWorkspace {
    Vec3 *aOld;
    Vec3 *aNew;
} GravityWorkspace;

typedef struct Planet {
    Vec3 position;     // km, float precision
    Vec3 velocity;     // km/s, float precision
    Vec3 acceleration; // km/s^2, float precision
    DATA mass;        // kg
    DATA radius;      // km
} Planet;

Planet create_planet(Vec3 position, Vec3 velocity, DATA mass, DATA radius);
void change_planet_position(Planet *planet, Vec3 dt, int add);
void planetGravityUpdate(Planet *bodies, int bodyCount, DATA dt, GravityWorkspace *ws);
DATA systemEnergy(const Planet *bodies, int bodyCount);

#endif