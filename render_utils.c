#include "render_utils.h"

void floatingOrigin(Vec3 *cam_pos, Planet *bodies, int num_bodies, float threshold)
{
    if(!cam_pos || !bodies)
        return;


    if(v3_len(*cam_pos) > threshold){
        for(int i = 0; i < num_bodies; i++)
            change_planet_position(&bodies[i], *cam_pos, 0); // subtract cam_pos from body position

        *cam_pos = v3_set(0.0f, 0.0f, 0.0f);
    }
}