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

void draw_rectangle(Vec3 p1, Vec3 p2, Vec3 p3, Vec3 p4, Vec3 color){
    draw_triangle(p1, p2, p3, color);
    draw_triangle(p1, p3, p4, color);
}

void draw_dot(const Vec3 position, const Vec3 color) {
    glBegin(GL_POINTS);
    glColor3f(color.x, color.y, color.z);
    glVertex3f(position.x, position.y, position.z);
    glEnd();
}
void draw_cuboid(const Vec3 position, float width, float length, float height, const Vec3 colors[]) {
    //glDisable(GL_CULL_FACE);
    Vec3 offset = v3_set(width / 2, length / 2, height / 2);

    /* bottom
      3-----4
      |     |
      2-----1

        top
      5-----6
      |     |
      8-----7
    */
    Vec3 c1 = v3_sub(position, offset);
    Vec3 c2 = v3_add(c1, v3_set(0,    length, 0));
    Vec3 c3 = v3_add(c1, v3_set(width, length, 0));
    Vec3 c4 = v3_add(c1, v3_set(width, 0,    0));

    Vec3 c5 = v3_add(position, offset);
    Vec3 c6 = v3_add(c5, v3_set(0,     -length, 0));
    Vec3 c7 = v3_add(c5, v3_set(-width, -length, 0));
    Vec3 c8 = v3_add(c5, v3_set(-width, 0,     0));

    draw_rectangle(c1, c2, c3, c4, colors[0]); // bottom
    draw_rectangle(c8, c7, c6, c5, colors[1]); // top

    draw_rectangle(c2, c1, c7, c8, colors[2]); // front
    draw_rectangle(c4, c3, c5, c6, colors[3]); // back

    draw_rectangle(c3, c2, c8, c5, colors[4]); // left
    draw_rectangle(c1, c4, c6, c7, colors[5]); // right
}

// draw a simple colored cube centered at position of given size (half-extent)
void draw_cube(const Vec3 position, float size, const Vec3 colors[]) {
    draw_cuboid(position, size, size, size, colors);
}