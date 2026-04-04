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

void planetGravityUpdate(Planet *body, int bodyCount, float dt) {
    if (!body) return;

    for(int i = 0; i < bodyCount; i++){
        body[i].acceleration = v3_set(0, 0, 0); // reset acceleration before calculating new one
        for(int j = 0; j < bodyCount; j++){
            if(i == j) continue;
            Vec3 relativeDistance = v3_sub(body[j].position, body[i].position);

            Vec3 acceleration = gravitationalAcceleration(body[j].mass, relativeDistance);

            body[i].acceleration = v3_add(body[i].acceleration, acceleration);
        }
    }

    
    for(int i = 0; i < bodyCount; i++){
        body[i].velocity = v3_add(body[i].velocity, v3_scale(body[i].acceleration, dt));
    }

    for(int i = 0; i < bodyCount; i++){
        body[i].position = v3_add(body[i].position, v3_scale(body[i].velocity, dt));
    }
}

double systemEnergy(const Planet* bodies, int bodyCount) {
    const double G = 6.67430e-11;

    double kinetic = 0.0;
    double potential = 0.0;

    // Kinetic energy: sum over all bodies
    for (int i = 0; i < bodyCount; ++i) {
        double speedSq = (double)v3_dot(bodies[i].velocity, bodies[i].velocity);
        kinetic += 0.5 * (double)bodies[i].mass * speedSq;
    }

    // Potential energy: sum each pair once
    for (int i = 0; i < bodyCount; ++i) {
        for (int j = i + 1; j < bodyCount; ++j) {
            Vec3 r = v3_sub(bodies[j].position, bodies[i].position);
            double distSq = (double)v3_dot(r, r);

            if (distSq == 0.0) {
                continue;
            }

            double distance = sqrt(distSq);
            potential += -G * (double)bodies[i].mass * (double)bodies[j].mass / distance;
        }
    }

    return kinetic + potential;
}