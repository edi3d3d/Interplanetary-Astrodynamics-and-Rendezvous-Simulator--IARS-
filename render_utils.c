#include "render_utils.h"
#include "planet.h"
#include "camera.h"
#include <math.h>

/* convert double world -> camera-space float for rendering */
Vec3 world_to_cam_f_d(const Vec3d world, const Vec3d cam_pos)
{
    return v3_set((float)(world.x - cam_pos.x),
                  (float)(world.y - cam_pos.y),
                  (float)(world.z - cam_pos.z));
}

/* Recenter world so camera stays near origin without visual jump.
   - cam must have a Vec3d position_d (authoritative double) and a float position (for rendering).
   - bodies[].position must be Vec3d (double).
   Call this once per frame (after camera_update and physics), before rendering. */
// ...existing code...
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