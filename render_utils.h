#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include "vec3.h"
#include "planet.h"
#include "camera.h"

void floatingOrigin_d(Camera *cam, Planet *bodies, int num_bodies, double threshold);

#endif // RENDER_UTILS_H