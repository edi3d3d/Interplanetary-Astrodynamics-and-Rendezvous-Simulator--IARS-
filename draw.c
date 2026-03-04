#include "draw.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    int stacks;
    int slices;
    GLuint list_id;
} SphereCache;

static SphereCache g_sphere = {0, 0, 0};

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

static GLuint build_unit_sphere_list(int stacks, int slices)
{
    const float TWO_PI = 2.0f * (float)M_PI;

    GLuint id = glGenLists(1);
    glNewList(id, GL_COMPILE);

    // Precompute azimuth sin/cos
    float *sinT = (float*)malloc((slices + 1) * sizeof(float));
    float *cosT = (float*)malloc((slices + 1) * sizeof(float));
    if (!sinT || !cosT) {
        free(sinT); free(cosT);
        glEndList();
        return id; // will be empty
    }

    for (int j = 0; j <= slices; ++j) {
        float theta = TWO_PI * (float)j / (float)slices;
        sinT[j] = sinf(theta);
        cosT[j] = cosf(theta);
    }

    // Draw a UNIT sphere centered at origin, radius = 1
    for (int i = 0; i < stacks; ++i) {
        float phi0 = (float)M_PI * ((float)i / (float)stacks - 0.5f);
        float phi1 = (float)M_PI * ((float)(i + 1) / (float)stacks - 0.5f);

        float cphi0 = cosf(phi0), sphi0 = sinf(phi0);
        float cphi1 = cosf(phi1), sphi1 = sinf(phi1);

        float r0 = cphi0;
        float y0 = sphi0;
        float r1 = cphi1;
        float y1 = sphi1;

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float x0 = r0 * cosT[j];
            float z0 = r0 * sinT[j];
            glVertex3f(x0, z0, y0);

            float x1 = r1 * cosT[j];
            float z1 = r1 * sinT[j];
            glVertex3f(x1, z1, y1);
        }
        glEnd();
    }

    free(sinT);
    free(cosT);

    glEndList();
    return id;
}

static void ensure_sphere_cache(int stacks, int slices)
{
    if (stacks < 2) stacks = 2;
    if (slices < 3) slices = 3;

    if (g_sphere.list_id && g_sphere.stacks == stacks && g_sphere.slices == slices)
        return;

    if (g_sphere.list_id) {
        glDeleteLists(g_sphere.list_id, 1);
        g_sphere.list_id = 0;
    }

    g_sphere.stacks = stacks;
    g_sphere.slices = slices;
    g_sphere.list_id = build_unit_sphere_list(stacks, slices);
}

// Call once at shutdown if you want to clean up explicitly
void draw_sphere_cache_destroy(void)
{
    if (g_sphere.list_id) {
        glDeleteLists(g_sphere.list_id, 1);
        g_sphere.list_id = 0;
    }
}

void draw_sphere(const Vec3 center, float radius, int stacks, int slices, const Vec3 colors)
{
    ensure_sphere_cache(stacks, slices);

    glPushMatrix();

    // Move + scale the cached UNIT sphere
    glTranslatef(center.x, center.y, center.z);
    glScalef(radius, radius, radius);

    glColor3f(colors.x, colors.y, colors.z);
    glCallList(g_sphere.list_id);

    glPopMatrix();
}