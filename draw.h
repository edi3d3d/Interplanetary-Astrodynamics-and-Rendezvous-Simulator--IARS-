#ifndef DRAW_H
#define DRAW_H

#include <stdio.h>
#include <stdlib.h>
#include "math.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "vec3.h"
// types used by helper functions
#include "planet.h"
#include "camera.h"

void draw_triangle(const Vec3 position1, const Vec3 position2, const Vec3 position3, const Vec3 color);
void draw_rectangle(const Vec3 position1, const Vec3 position2, const Vec3 position3, const Vec3 position4, const Vec3 color);
void draw_dot(const Vec3 position, const Vec3 color);
void draw_cuboid(const Vec3 position, float width, float length, float height, const Vec3 colors[]);
void draw_cube(const Vec3 position, float size, const Vec3 colors[]);
void draw_sphere(const Vec3 center, float radius, int stacks, int slices, const Vec3 colors);
void draw_arrow(float x, float y, float dir_x, float dir_y, Vec3 color);


int world_to_screen_visible(const Vec3 world, int win_w, int win_h, float *sx, float *sy, float *sz);

// Helpers for on-screen / off-screen rendering of planets
// Returns 1 if planet is visible on-screen and in front of camera. sx/sy are set to screen coords.
int visible_on_screen_and_in_front(const Planet planet, const Camera *cam, int win_w, int win_h, float *sx, float *sy);

// Draws either the planet (when visible) or an offscreen indicator. 'margin' is pixels inset from window edge.
void draw_planet_or_indicator(const Planet planet, const Camera *cam, int win_w, int win_h,
                              int stacks, int slices, const Vec3 colors, float margin);


// place indicator at edge along direction from screen center
void draw_offscreen_indicator_from_dir(float dir_x, float dir_y, int w, int h, float margin, Vec3 color);

#endif