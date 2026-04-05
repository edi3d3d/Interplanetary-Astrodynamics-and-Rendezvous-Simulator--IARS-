#include "planet.h"
#include <stdlib.h>
#include <math.h>

Planet create_planet(Vec3 position, Vec3 velocity, DATA mass, DATA radius) {
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

static void computeAccelerations(const Planet *bodies, Vec3 *acc, int bodyCount) {
    const DATA G = 6.67430e-11f;
    const DATA eps = 1e-3f;
    const DATA eps2 = eps * eps;

    for (int i = 0; i < bodyCount; ++i) {
        acc[i].x = 0.0f;
        acc[i].y = 0.0f;
        acc[i].z = 0.0f;
    }

    for (int i = 0; i < bodyCount; ++i) {
        const DATA ix = bodies[i].position.x;
        const DATA iy = bodies[i].position.y;
        const DATA iz = bodies[i].position.z;
        const DATA mi = bodies[i].mass;

        for (int j = i + 1; j < bodyCount; ++j) {
            const DATA rx = bodies[j].position.x - ix;
            const DATA ry = bodies[j].position.y - iy;
            const DATA rz = bodies[j].position.z - iz;

            const DATA distSq = rx * rx + ry * ry + rz * rz + eps2;
            const DATA invDist = 1.0f / sqrtf(distSq);
            const DATA invDist3 = invDist * invDist * invDist;

            const DATA scaleI = G * bodies[j].mass * invDist3;
            const DATA scaleJ = G * mi * invDist3;

            acc[i].x += rx * scaleI;
            acc[i].y += ry * scaleI;
            acc[i].z += rz * scaleI;

            acc[j].x -= rx * scaleJ;
            acc[j].y -= ry * scaleJ;
            acc[j].z -= rz * scaleJ;
        }
    }
}

void planetGravityUpdate(Planet *bodies, int bodyCount, DATA dt, GravityWorkspace *ws) {
    //TODO: Yoshida 4th-order symplectic integrator
    if (!bodies || !ws || bodyCount <= 0 || bodyCount > MAX_BODIES || dt <= 0.0f) {
        return;
    }

    const DATA maxStep = 1.0f;
    int steps = (int)ceilf(dt / maxStep);
    if (steps < 1) {
        steps = 1;
    }

    const DATA stepDt = dt / (DATA)steps;
    const DATA halfDt2 = 0.5f * stepDt * stepDt;
    const DATA halfDt = 0.5f * stepDt;

    for (int s = 0; s < steps; ++s) {
        computeAccelerations(bodies, ws->aOld, bodyCount);

        for (int i = 0; i < bodyCount; ++i) {
            bodies[i].position.x += bodies[i].velocity.x * stepDt + ws->aOld[i].x * halfDt2;
            bodies[i].position.y += bodies[i].velocity.y * stepDt + ws->aOld[i].y * halfDt2;
            bodies[i].position.z += bodies[i].velocity.z * stepDt + ws->aOld[i].z * halfDt2;
        }

        computeAccelerations(bodies, ws->aNew, bodyCount);

        for (int i = 0; i < bodyCount; ++i) {
            bodies[i].velocity.x += (ws->aOld[i].x + ws->aNew[i].x) * halfDt;
            bodies[i].velocity.y += (ws->aOld[i].y + ws->aNew[i].y) * halfDt;
            bodies[i].velocity.z += (ws->aOld[i].z + ws->aNew[i].z) * halfDt;

            bodies[i].acceleration = ws->aNew[i];
        }
    }
}

DATA systemEnergy(const Planet* bodies, int bodyCount) {
    const DATA G = 6.67430e-11;

    DATA kinetic = 0.0;
    DATA potential = 0.0;

    // Kinetic energy: sum over all bodies
    for (int i = 0; i < bodyCount; ++i) {
        DATA speedSq = (DATA)v3_dot(bodies[i].velocity, bodies[i].velocity);
        kinetic += 0.5 * (DATA)bodies[i].mass * speedSq;
    }

    // Potential energy: sum each pair once
    for (int i = 0; i < bodyCount; ++i) {
        for (int j = i + 1; j < bodyCount; ++j) {
            Vec3 r = v3_sub(bodies[j].position, bodies[i].position);
            DATA distSq = (DATA)v3_dot(r, r);

            if (distSq == 0.0) {
                continue;
            }

            DATA distance = sqrt(distSq);
            potential += -G * (DATA)bodies[i].mass * (DATA)bodies[j].mass / distance;
        }
    }

    return kinetic + potential;
}