#include "render_utils.h"
#include <math.h>


// Recenter world so camera stays near origin without visual jump.
void floatingOrigin_d(Camera *cam, Planet *bodies, int num_bodies, double threshold)
{
    if (!cam || !bodies || num_bodies <= 0) return;

    /* use squared length to avoid sqrt and compare squares consistently */

    if (v3_len(cam->position) <= threshold) return;

    for (int i = 0; i < num_bodies; ++i) {
        bodies[i].position.x -= cam->position.x;
        bodies[i].position.y -= cam->position.y;
        bodies[i].position.z -= cam->position.z;
    }

    cam->position = v3_set(0.0f, 0.0f, 0.0f);
}

