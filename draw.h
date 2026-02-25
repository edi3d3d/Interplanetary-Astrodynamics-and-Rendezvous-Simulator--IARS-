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

void draw_triangle(const Vec3 position1, const Vec3 position2, const Vec3 position3, const Vec3 color);
void draw_square(const Vec3 position1, const Vec3 position2, const Vec3 position3, const Vec3 position4, const Vec3 color);
void draw_dot(const Vec3 position, const Vec3 color);
void draw_cube(const Vec3 position, float size);
void draw_sphere(const Vec3 center, float radius, int stacks, int slices, const Vec3 color);

#endif