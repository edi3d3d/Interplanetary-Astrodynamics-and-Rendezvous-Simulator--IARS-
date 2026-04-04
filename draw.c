#include "draw.h"


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

    Vec3 default_colors[6];
    if(colors == NULL) {
        default_colors[0] = v3_set(1, 0, 0); // red
        default_colors[1] = v3_set(0, 1, 0); // green
        default_colors[2] = v3_set(0, 0, 1); // blue
        default_colors[3] = v3_set(1, 1, 0); // yellow
        default_colors[4] = v3_set(1, 0, 1); // magenta
        default_colors[5] = v3_set(0, 1, 1); // cyan
    } else {
        for (int i = 0; i < 6; ++i) {
            default_colors[i] = colors[i];
        }
    }

    draw_rectangle(c1, c2, c3, c4, default_colors[0]); // bottom
    draw_rectangle(c8, c7, c6, c5, default_colors[1]); // top

    draw_rectangle(c2, c1, c7, c8, default_colors[2]); // front
    draw_rectangle(c4, c3, c5, c6, default_colors[3]); // back

    draw_rectangle(c3, c2, c8, c5, default_colors[4]); // left
    draw_rectangle(c1, c4, c6, c7, default_colors[5]); // right
}

// draw a simple colored cube centered at position of given size (half-extent)
void draw_cube(const Vec3 position, float size, const Vec3 colors[]) {
    draw_cuboid(position, size, size, size, colors);
}

static GLuint build_unit_sphere_list(int stacks, int slices)
{
    const float PI = (float)M_PI;
    const float TWO_PI = 2.0f * PI;

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

    // Generate sphere as quads (two triangles each) with consistent CCW winding
    for (int i = 0; i < stacks; ++i) {
        float phi0 = PI * ((float)i / (float)stacks - 0.5f);
        float phi1 = PI * ((float)(i + 1) / (float)stacks - 0.5f);

        float cphi0 = cosf(phi0), sphi0 = sinf(phi0);
        float cphi1 = cosf(phi1), sphi1 = sinf(phi1);

        float r0 = cphi0;
        float y0 = sphi0;
        float r1 = cphi1;
        float y1 = sphi1;

        // For each slice, emit two triangles forming a quad with consistent winding
        for (int j = 0; j < slices; ++j) {
            int j1 = j;
            int j2 = (j + 1);

            float x00 = r0 * cosT[j1]; float z00 = r0 * sinT[j1]; // v00
            float x01 = r0 * cosT[j2]; float z01 = r0 * sinT[j2]; // v01
            float x10 = r1 * cosT[j1]; float z10 = r1 * sinT[j1]; // v10
            float x11 = r1 * cosT[j2]; float z11 = r1 * sinT[j2]; // v11

            // Triangle 1: v00, v10, v11
            glBegin(GL_TRIANGLES);
            glVertex3f(x00, y0, z00);
            glVertex3f(x10, y1, z10);
            glVertex3f(x11, y1, z11);
            // Triangle 2: v00, v11, v01
            glVertex3f(x00, y0, z00);
            glVertex3f(x11, y1, z11);
            glVertex3f(x01, y0, z01);
            glEnd();
        }
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

    // Sphere mesh generated with consistent CCW winding; enable culling globally
    // to let the GPU discard backfaces for performance.
    glColor3f(colors.x, colors.y, colors.z);
    glCallList(g_sphere.list_id);

    glPopMatrix();
}

void draw_arrow(float x, float y, float dir_x, float dir_y, Vec3 color)
{
    // Draw a small 2D arrow at screen pixel (x,y) pointing along (dir_x, dir_y).
    // Use display DPI to approximate 1 cm length, fall back to 96 DPI.
    GLint view[4];
    glGetIntegerv(GL_VIEWPORT, view);
    int win_w = view[2];
    int win_h = view[3];

    float dpi = 96.0f; // default
#ifdef SDL_VERSION_ATLEAST
    {
        float ddpi, hdpi, vdpi;
        if (SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi) == 0) {
            dpi = ddpi > 0.0f ? ddpi : dpi;
        }
    }
#endif

    const float px_per_cm = dpi / 2.54f;
    const float length_px = px_per_cm * 1.0f; // ~1 cm arrow
    const float head_fraction = 0.35f; // fraction of length for arrow head
    const float head_width = length_px * 0.5f; // width of arrow head
    const float shaft_length = length_px * (1.0f - head_fraction);

    // normalize direction
    float len = sqrtf(dir_x*dir_x + dir_y*dir_y);
    if (len < 1e-6f) return;
    float nx = dir_x / len;
    float ny = dir_y / len;
    // perp vector (to the right of direction)
    float px = -ny;
    float py = nx;

    // coordinates: tip at (x + nx*length_px, y + ny*length_px)
    float tip_x = x + nx * length_px;
    float tip_y = y + ny * length_px;
    // base center of head
    float base_x = x + nx * shaft_length;
    float base_y = y + ny * shaft_length;

    float left_x = base_x + px * (head_width * 0.5f);
    float left_y = base_y + py * (head_width * 0.5f);
    float right_x = base_x - px * (head_width * 0.5f);
    float right_y = base_y - py * (head_width * 0.5f);

    // shaft start
    float shaft_start_x = x - nx * (length_px * 0.1f);
    float shaft_start_y = y - ny * (length_px * 0.1f);

    // Save GL state we will change
    GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Setup orthographic projection for screen-space coords with origin top-left
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, win_w, win_h, 0, -1, 1); // top-left origin

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Arrow color (tunable)
    glColor4f(1.0f, 0.65f, 0.0f, 0.95f); // orange

    // Draw shaft
    glLineWidth(2.0f);
    glColor4f(color.x, color.y, color.z, 0.95f);
    glBegin(GL_LINES);
    glVertex2f(shaft_start_x, shaft_start_y);
    glVertex2f(base_x, base_y);
    glEnd();

    // Draw filled triangular head
    glBegin(GL_TRIANGLES);
    glVertex2f(tip_x, tip_y);
    glVertex2f(left_x, left_y);
    glVertex2f(right_x, right_y);
    glEnd();

    // optional outline for contrast
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(tip_x, tip_y);
    glVertex2f(left_x, left_y);
    glVertex2f(right_x, right_y);
    glEnd();

    // Restore matrices and state
    glPopMatrix(); // MODELVIEW
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    if (depthEnabled) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    glPopAttrib();
}


// Project world coordinates to window/screen coordinates using current GL matrices.
// Returns 1 on success (out_x/out_y/out_z set), 0 on failure.
int project_world_to_screen(const Vec3 world, int win_w, int win_h, float *out_x, float *out_y, float *out_z)
{
    GLdouble model[16], proj[16];
    GLint view[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, view);

    GLdouble wx, wy, wz;
    if (!gluProject((GLdouble)world.x, (GLdouble)world.y, (GLdouble)world.z,
                    model, proj, view, &wx, &wy, &wz)) return 0;

    // convert from GL bottom-origin to top-left origin
    wy = (GLdouble)view[3] - wy;

    *out_x = (float)wx;
    *out_y = (float)wy;
    *out_z = (float)wz;
    return 1;
}


// float world/x,y,z and Camera already applied to GL_MODELVIEW
// Returns 1 if visible on-screen (and in front), 0 otherwise.
// sx, sy = window coords; sz = depth [0..1]
int world_to_screen_visible(const Vec3 world, int win_w, int win_h, float *sx, float *sy, float *sz)
{
    GLdouble model[16], proj[16];
    GLint   view[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, view);

    GLdouble wx, wy, wz;
    if (!gluProject((GLdouble)world.x, (GLdouble)world.y, (GLdouble)world.z,
                    model, proj, view, &wx, &wy, &wz)) return 0;

    // gluProject returns window Y with origin at bottom; convert to top-left if needed
    wy = (GLdouble)view[3] - wy;

    *sx = (float)wx;
    *sy = (float)wy;
    *sz = (float)wz;

    // visible if inside viewport AND depth in [0,1]
    if (wx >= 0.0 && wx <= (GLdouble)win_w &&
        wy >= 0.0 && wy <= (GLdouble)win_h &&
        wz >= 0.0 && wz <= 1.0) {
        return 1;
    }
    return 0;
}


// place indicator at edge along direction from screen center
void draw_offscreen_indicator_from_dir(float dir_x, float dir_y, int w, int h, float margin, Vec3 color)
{
    float cx = w*0.5f, cy = h*0.5f;
    // normalize direction on 2D plane
    float len = sqrtf(dir_x*dir_x + dir_y*dir_y);
    if (len < 1e-6f) return;
    float nx = dir_x / len, ny = dir_y / len;

    // max radius inside viewport minus margin
    float rx = cx - margin, ry = cy - margin;
    // compute t to hit horizontal/vertical edges and choose smallest positive
    float tx = (nx > 0) ? (rx / nx) : (-rx / nx);
    float ty = (ny > 0) ? (ry / ny) : (-ry / ny);
    float t = fminf(fabsf(tx), fabsf(ty));

    float ix = cx + nx * t;
    float iy = cy + ny * t;

    
    draw_arrow(ix, iy, nx, ny, color);
    // draw your arrow/indicator at (ix, iy) pointing along (nx, ny)
}


// Return 1 if planet is visible on-screen and in front of camera. sx/sy set to window coords.
int visible_on_screen_and_in_front(const Planet planet, const Camera *cam, int win_w, int win_h, float *sx, float *sy)
{
    if (!cam) return 0;
    float sz;
    Vec3 world = planet.position; // planet positions are in km floats
    int vis = world_to_screen_visible(world, win_w, win_h, sx, sy, &sz);
    // world_to_screen_visible returns 1 if on-screen and in front (sz between 0..1) typically
    // ensure depth is in front (sz between 0 and 1)
    if (!vis) return 0;
    if (sz < 0.0f || sz > 1.0f) return 0;
    return 1;
}


void draw_planet_or_indicator(const Planet planet, const Camera *cam, int win_w, int win_h,
                              int stacks, int slices, const Vec3 colors, float margin)
{
    if (!cam) return;
    float sx, sy;
    if (visible_on_screen_and_in_front(planet, cam, win_w, win_h, &sx, &sy)){
        draw_sphere(planet.position, planet.radius, stacks, slices, colors);
        draw_dot(planet.position, colors); // optional: draw center dot for visibility
    } else {
        // Prefer to compute direction in screen-space: project world point even if offscreen
        float wx, wy, wz;
        float dir_x, dir_y;
        int proj_ok = project_world_to_screen(planet.position, win_w, win_h, &wx, &wy, &wz);
        float cx = win_w * 0.5f;
        float cy = win_h * 0.5f;
        if (proj_ok) {
            dir_x = wx - cx;
            dir_y = wy - cy;
        } else {
            // fallback: approximate with world-space delta in camera-local XY plane
            Vec3 dir = v3_sub(planet.position, cam->position);
            dir_x = dir.x;
            dir_y = dir.y;
        }
        // draw indicator at screen edge pointing along the screen-space direction towards the planet
        draw_offscreen_indicator_from_dir(dir_x, dir_y, win_w, win_h, margin, colors);
    }
}