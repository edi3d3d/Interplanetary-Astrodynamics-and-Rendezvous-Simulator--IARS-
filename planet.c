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
    const SMALL_DATA G = 6.67430e-11f;
    const SMALL_DATA eps = 1e-3f;
    const SMALL_DATA eps2 = eps * eps;

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

            SMALL_DATA srx = (SMALL_DATA)rx;
            SMALL_DATA sry = (SMALL_DATA)ry;
            SMALL_DATA srz = (SMALL_DATA)rz;

            SMALL_DATA distSq = srx * srx + sry * sry + srz * srz + eps2;
            SMALL_DATA invDist = 1.0f / sqrt(distSq);
            SMALL_DATA invDist3 = invDist * invDist * invDist;

            SMALL_DATA scaleI = G * (SMALL_DATA)bodies[j].mass * invDist3;
            SMALL_DATA scaleJ = G * (SMALL_DATA)mi * invDist3;

            acc[i].x += (DATA)(srx * scaleI);
            acc[i].y += (DATA)(sry * scaleI);
            acc[i].z += (DATA)(srz * scaleI);

            acc[j].x -= (DATA)(srx * scaleJ);
            acc[j].y -= (DATA)(sry * scaleJ);
            acc[j].z -= (DATA)(srz * scaleJ);
        }
    }
}

void planetGravityUpdate(Planet *bodies, int bodyCount, DATA dt, GravityWorkspace *ws) {
    //TODO: Yoshida 4th-order symplectic integrator
    if (!bodies || !ws || bodyCount <= 0 || bodyCount > MAX_BODIES || dt <= 0.0f) {
        return;
    }

    const SMALL_DATA maxStep = 1.0f;
    int steps = (int)ceil((double)dt / (double)maxStep);
    if (steps < 1) {
        steps = 1;
    }

    const SMALL_DATA stepDt = (SMALL_DATA)dt / (SMALL_DATA)steps;
    const SMALL_DATA halfDt2 = 0.5f * stepDt * stepDt;
    const SMALL_DATA halfDt = 0.5f * stepDt;

    for (int s = 0; s < steps; ++s) {
        computeAccelerations(bodies, ws->aOld, bodyCount);

        for (int i = 0; i < bodyCount; ++i) {
            /* velocity and accel buffers are DATA; treat them as SMALL_DATA for stepping */
            SMALL_DATA vx = (SMALL_DATA)bodies[i].velocity.x;
            SMALL_DATA vy = (SMALL_DATA)bodies[i].velocity.y;
            SMALL_DATA vz = (SMALL_DATA)bodies[i].velocity.z;

            SMALL_DATA ax = (SMALL_DATA)ws->aOld[i].x;
            SMALL_DATA ay = (SMALL_DATA)ws->aOld[i].y;
            SMALL_DATA az = (SMALL_DATA)ws->aOld[i].z;

            SMALL_DATA dx = vx * stepDt + ax * halfDt2;
            SMALL_DATA dy = vy * stepDt + ay * halfDt2;
            SMALL_DATA dz = vz * stepDt + az * halfDt2;

            bodies[i].position.x += (DATA)dx;
            bodies[i].position.y += (DATA)dy;
            bodies[i].position.z += (DATA)dz;
        }

        computeAccelerations(bodies, ws->aNew, bodyCount);

        for (int i = 0; i < bodyCount; ++i) {
            SMALL_DATA aoldx = (SMALL_DATA)ws->aOld[i].x;
            SMALL_DATA aoldy = (SMALL_DATA)ws->aOld[i].y;
            SMALL_DATA aoldz = (SMALL_DATA)ws->aOld[i].z;

            SMALL_DATA anewx = (SMALL_DATA)ws->aNew[i].x;
            SMALL_DATA anewy = (SMALL_DATA)ws->aNew[i].y;
            SMALL_DATA anewz = (SMALL_DATA)ws->aNew[i].z;

            bodies[i].velocity.x += (DATA)((aoldx + anewx) * halfDt);
            bodies[i].velocity.y += (DATA)((aoldy + anewy) * halfDt);
            bodies[i].velocity.z += (DATA)((aoldz + anewz) * halfDt);

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