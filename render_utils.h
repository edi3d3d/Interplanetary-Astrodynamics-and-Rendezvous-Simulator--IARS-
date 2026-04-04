#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include "vec3.h"
#include "planet.h"
#include "camera.h"

typedef struct Vec3d { double x,y,z; } Vec3d;

void floatingOrigin_d(Camera *cam, Planet *bodies, int num_bodies, double threshold);
Vec3 world_to_cam_f_d(const Vec3d world, const Vec3d cam_pos);

void draw_offscreen_indicator_from_dir(float dir_x, float dir_y, int w, int h, float margin);

#endif // RENDER_UTILS_H