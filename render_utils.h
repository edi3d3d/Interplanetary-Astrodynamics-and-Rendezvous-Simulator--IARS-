#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include "vec3.h"
#include "planet.h"

/* Convert world coordinates based on camera position compared to a threshold */
void floatingOrigin(Vec3 *cam_pos, Planet *bodies, int num_bodies, float threshold);

#endif // RENDER_UTILS_H