#include "draw.h"
#define M_PI 3.14159265358979323846

void draw_triangle(const Vec3 position1, const Vec3 position2, const Vec3 position3, const Vec3 color) {
    glBegin(GL_TRIANGLES);
    glColor3f(color.x, color.y, color.z);
    glVertex3f(position1.x, position1.y, position1.z);
    glVertex3f(position2.x, position2.y, position2.z);
    glVertex3f(position3.x, position3.y, position3.z);
    glEnd();
}

void draw_square(Vec3 p1, Vec3 p2, Vec3 p3, Vec3 p4, Vec3 color) {
    draw_triangle(p1, p2, p3, color);
    draw_triangle(p1, p3, p4, color);
}

void draw_dot(const Vec3 position, const Vec3 color) {
    glBegin(GL_POINTS);
    glColor3f(color.x, color.y, color.z);
    glVertex3f(position.x, position.y, position.z);
    glEnd();
}

// draw a simple colored cube centered at position of given size (half-extent)
void draw_cube(const Vec3 position, float size) {

    //glDisable(GL_CULL_FACE);
    Vec3 offset = v3_set(size / 2, size / 2, size / 2);

    /* bottom
      3-----4
      |     |
      |     |
      2-----1

        top
      5-----6
      |     |
      |     |
      8-----7
    */
    Vec3 c1 = v3_sub(position, offset);
    Vec3 c2 = v3_add(c1, v3_set(0,    size, 0));
    Vec3 c3 = v3_add(c1, v3_set(size, size, 0));
    Vec3 c4 = v3_add(c1, v3_set(size, 0,    0));


    Vec3 c5 = v3_add(position, offset);
    Vec3 c6 = v3_add(c5, v3_set(0,     -size, 0));
    Vec3 c7 = v3_add(c5, v3_set(-size, -size, 0));
    Vec3 c8 = v3_add(c5, v3_set(-size, 0,     0));

    draw_dot(position, v3_set(1,1,1)); // center dot

    draw_square(c1, c2, c3, c4, v3_set(1,0,0)); // bottom
    draw_square(c8, c7, c6, c5, v3_set(1,1,0)); // top

    draw_square(c2, c1, c7, c8, v3_set(0,1,0)); // front
    draw_square(c4, c3, c5, c6, v3_set(1,1,1)); // back

    draw_square(c3, c2, c8, c5, v3_set(0,1,1)); // left
    draw_square(c1, c4, c6, c7, v3_set(0,0,1)); // right
}