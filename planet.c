#include "planet.h"
#include "vec3.h"

Planet create_planet(Vec3 position, Vec3 velocity, float mass, float radius) {
    Planet planet;
    planet.position = position;
    planet.velocity = velocity;
    planet.mass = mass;
    planet.radius = radius;
    return planet;
}