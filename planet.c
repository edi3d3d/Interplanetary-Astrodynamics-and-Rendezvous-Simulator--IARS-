#include "planet.h"

Planet create_planet(Vec3 position, Vec3 velocity, float mass, float radius) {
    Planet planet;
    planet.position = position;
    planet.velocity = velocity;
    planet.mass = mass;
    planet.radius = radius;
    return planet;
}

void change_planet_position(Planet *planet, Vec3 dt, int add) {
    
    if(!planet)
        return;

    if (add) {
        planet->position = v3_add(planet->position, dt);
    } else {
        planet->position = v3_sub(planet->position, dt);
    }
}