#include "camera.h"
#include "vec3.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef USE_SDL_TTF
#include <SDL2/SDL_ttf.h>
#endif

static void update_orbital_position(Camera *cam)
{
    if (!cam) return;
    // compute spherical offset from yaw/pitch/distance
    float cp = cosf(cam->rot.x); // pitch
    float sp = sinf(cam->rot.x);
    float ox = cam->distance * cp * sinf(cam->rot.y); // yaw
    float oy = cam->distance * sp;
    float oz = cam->distance * cp * cosf(cam->rot.y);

    cam->position.x = cam->target.x + ox;
    cam->position.y = cam->target.y + oy;
    cam->position.z = cam->target.z + oz;
}

// compute camera axes using vec3 helpers
static void compute_camera_axes_vec(const Camera *cam, Vec3 *forward, Vec3 *right, Vec3 *up)
{
    Vec3 worldUp; v3_set(&worldUp, 0.0f, 1.0f, 0.0f);

    if (cam->freeMode) {
        // forward from yaw/pitch (rot.x = pitch, rot.y = yaw)
        v3_set(forward,
               cosf(cam->rot.x) * sinf(cam->rot.y),
               sinf(cam->rot.x),
               cosf(cam->rot.x) * cosf(cam->rot.y));
    } else {
        // orbital: forward = target - position
        v3_set(forward,
               cam->target.x - cam->position.x,
               cam->target.y - cam->position.y,
               cam->target.z - cam->position.z);
    }
    v3_normalize(forward, forward);

    // right = cross(forward, worldUp)
    v3_cross(right, forward, &worldUp);
    v3_normalize(right, right);

    // up = cross(right, forward)
    v3_cross(up, right, forward);
    v3_normalize(up, up);

    // apply roll: rotate (right, up) around forward by cam->rot.z
    if (cam->rot.z != 0.0f) {
        float c = cosf(cam->rot.z);
        float s = sinf(cam->rot.z);
        Vec3 newRight, newUp;
        // Correct rotation around forward (right,up) basis:
        // newRight = right * c - up * s
        v3_scale(&newRight, right, c);
        v3_add_scaled(&newRight, &newRight, up, -s); // newRight += up * (-s)
        v3_normalize(&newRight, &newRight);
        // newUp = right * s + up * c
        v3_scale(&newUp, right, s);
        v3_add_scaled(&newUp, &newUp, up, c);
        v3_normalize(&newUp, &newUp);
        v3_copy(right, &newRight);
        v3_copy(up, &newUp);
    }
}

void camera_init(Camera *cam)
{
    if (!cam) return;
    v3_set(&cam->target, 0.0f, 0.0f, 0.0f);

    cam->rot.x = 0.2f; // pitch default
    cam->rot.y = 0.0f; // yaw default
    cam->rot.z = 0.0f; // roll default
    cam->distance = 5.0f;

    cam->dragging = 0;
    cam->lastMouseX = cam->lastMouseY = 0;

    cam->moveSpeed = 5.0f; // units per second
    cam->sensitivityX = 0.005f;
    cam->sensitivityY = 0.005f;
    cam->mouseRotateSpeed = 1.0f;
    cam->zoomSpeed = 0.1f;

    cam->freeMode = 0; // start in orbit mode

    // initialize position from orbital parameters
    update_orbital_position(cam);

    // initialize local/base positions
    v3_set(&cam->localPosition, 0.0f, 0.0f, 0.0f);
    v3_copy(&cam->basePosition, &cam->position);
}

void camera_handle_event(Camera *cam, const SDL_Event *e)
{
    if (!cam || !e) return;
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN:
        if (e->button.button == SDL_BUTTON_LEFT) {
            cam->dragging = 1;
            cam->lastMouseX = e->button.x;
            cam->lastMouseY = e->button.y;
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if (e->button.button == SDL_BUTTON_LEFT) {
            cam->dragging = 0;
        }
        break;
    case SDL_MOUSEMOTION:
        if (cam->dragging) {
            int dx = e->motion.x - cam->lastMouseX;
            int dy = e->motion.y - cam->lastMouseY;
            cam->rot.y   -= dx * cam->sensitivityX * cam->mouseRotateSpeed; // yaw
            cam->rot.x += dy * cam->sensitivityY * cam->mouseRotateSpeed; // pitch
            // clamp pitch to avoid flipping
            const float maxPitch = 1.49f; // ~85 degrees
            if (cam->rot.x > maxPitch) cam->rot.x = maxPitch;
            if (cam->rot.x < -maxPitch) cam->rot.x = -maxPitch;
            cam->lastMouseX = e->motion.x;
            cam->lastMouseY = e->motion.y;

            // when rotating, update orbital position so cam->position reflects view
            if (!cam->freeMode) update_orbital_position(cam);
        }
        break;
    case SDL_MOUSEWHEEL:
        if (e->wheel.y > 0) {
            if (cam->freeMode) {
                Vec3 f; v3_set(&f, cosf(cam->rot.x) * sinf(cam->rot.y), sinf(cam->rot.x), cosf(cam->rot.x) * cosf(cam->rot.y));
                v3_normalize(&f, &f);
                cam->position.x += f.x * cam->zoomSpeed * cam->distance;
                cam->position.y += f.y * cam->zoomSpeed * cam->distance;
                cam->position.z += f.z * cam->zoomSpeed * cam->distance;
            } else {
                cam->distance *= (1.0f - cam->zoomSpeed);
                if (cam->distance < 0.1f) cam->distance = 0.1f;
                update_orbital_position(cam);
            }
        } else if (e->wheel.y < 0) {
            if (cam->freeMode) {
                Vec3 f; v3_set(&f, cosf(cam->rot.x) * sinf(cam->rot.y), sinf(cam->rot.x), cosf(cam->rot.x) * cosf(cam->rot.y));
                v3_normalize(&f, &f);
                cam->position.x -= f.x * cam->zoomSpeed * cam->distance;
                cam->position.y -= f.y * cam->zoomSpeed * cam->distance;
                cam->position.z -= f.z * cam->zoomSpeed * cam->distance;
            } else {
                cam->distance *= (1.0f + cam->zoomSpeed);
                if (cam->distance > 100.0f) cam->distance = 100.0f;
                update_orbital_position(cam);
            }
        }
        break;
    default:
        break;
    }
}

// keyboardState comes from SDL_GetKeyboardState(NULL)
void camera_update(Camera *cam, const Uint8 *keyboardState, float dt)
{
    if (!cam || !keyboardState) return;

    int moving = 0;
    float moveF = 0.0f; // forward/back
    float moveR = 0.0f; // right/left
    float moveU = 0.0f; // up/down

    if (keyboardState[SDL_SCANCODE_W]) { moveF += 1.0f; moving = 1; }
    if (keyboardState[SDL_SCANCODE_S]) { moveF -= 1.0f; moving = 1; }
    if (keyboardState[SDL_SCANCODE_D]) { moveR += 1.0f; moving = 1; }
    if (keyboardState[SDL_SCANCODE_A]) { moveR -= 1.0f; moving = 1; }
    if (keyboardState[SDL_SCANCODE_LSHIFT] || keyboardState[SDL_SCANCODE_RSHIFT]) { moveU += 1.0f; moving = 1; }
    if (keyboardState[SDL_SCANCODE_LCTRL] || keyboardState[SDL_SCANCODE_RCTRL]) { moveU -= 1.0f; moving = 1; }

    // Q/E roll (Q = roll left, E = roll right)
    const float rollSpeed = 1.5f; // rad/s
    if (keyboardState[SDL_SCANCODE_Q]) cam->rot.z += rollSpeed * dt;
    if (keyboardState[SDL_SCANCODE_E]) cam->rot.z -= rollSpeed * dt;

    if (moving) {
        Vec3 forward, right, up;
        compute_camera_axes_vec(cam, &forward, &right, &up);

        // build world-space delta from local axes
        Vec3 delta; v3_set(&delta, 0,0,0);
        // delta = right * moveR + forward * moveF + up * moveU
        v3_add_scaled(&delta, &delta, &right, moveR);
        v3_add_scaled(&delta, &delta, &forward, moveF);
        v3_add_scaled(&delta, &delta, &up, moveU);

        float dlen = v3_len(&delta);
        if (dlen > 1e-6f) {
            v3_scale(&delta, &delta, (cam->moveSpeed * dt) / dlen);

            if (cam->freeMode) {
                cam->position.x += delta.x;
                cam->position.y += delta.y;
                cam->position.z += delta.z;
            } else {
                cam->target.x += delta.x;
                cam->target.y += delta.y;
                cam->target.z += delta.z;
                update_orbital_position(cam);
            }
        }
    }
}

void camera_apply_view(const Camera *cam)
{
    if (!cam) return;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    Vec3 forward, right, up;
    compute_camera_axes_vec(cam, &forward, &right, &up);

    float eyeX = cam->position.x;
    float eyeY = cam->position.y;
    float eyeZ = cam->position.z;

    if (cam->freeMode) {
        float centerX = eyeX + forward.x;
        float centerY = eyeY + forward.y;
        float centerZ = eyeZ + forward.z;
        gluLookAt(eyeX, eyeY, eyeZ,
                  centerX, centerY, centerZ,
                  up.x, up.y, up.z);
    } else {
        gluLookAt(eyeX, eyeY, eyeZ,
                  cam->target.x, cam->target.y, cam->target.z,
                  up.x, up.y, up.z);
    }
}

void camera_draw_coordinates(const Camera *cam, SDL_Window *window)
{
    if (!cam || !window) return;

    const float RAD2DEG = 57.29577951308232f; // 180 / PI
    float yawDeg = fmodf(cam->rot.y * RAD2DEG, 360.0f);
    if (yawDeg < 0.0f) yawDeg += 360.0f;
    float pitchDeg = fmodf(cam->rot.x * RAD2DEG, 360.0f);
    if (pitchDeg < 0.0f) pitchDeg += 360.0f;
    float rollDeg = fmodf(cam->rot.z * RAD2DEG, 360.0f);
    if (rollDeg < 0.0f) rollDeg += 360.0f;

    char buf[256];
    snprintf(buf, sizeof(buf), "Pos: %.2f, %.2f, %.2f  Rot(p,y,r): %.1f, %.1f, %.1f deg",
             cam->position.x, cam->position.y, cam->position.z,
             pitchDeg, yawDeg, rollDeg);

#ifdef USE_SDL_TTF
    // Render text using SDL_ttf and draw as an OpenGL texture in the top-right corner
    static TTF_Font *font = NULL;
    if (!font) {
        if (TTF_WasInit() == 0) TTF_Init();
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 14);
        if (!font) {
            fprintf(stderr, "Failed to load font for camera_draw_coordinates\n");
        }
    }
    if (font) {
        SDL_Color white = {255,255,255,255};
        SDL_Surface *surf = TTF_RenderText_Blended(font, buf, white);
        if (!surf) return;

        // create GL texture
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLenum format = (surf->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, surf->w, surf->h, 0, format, GL_UNSIGNED_BYTE, surf->pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int winW, winH;
        SDL_GetWindowSize(window, &winW, &winH);

        // draw textured quad in top-right corner using orthographic projection
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, winW, 0, winH, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        glColor3f(1.0f, 1.0f, 1.0f);

        float x = (float)(winW - surf->w - 8);
        float y = (float)(winH - surf->h - 8);

        glBegin(GL_QUADS);
        glTexCoord2f(0,1); glVertex2f(x, y);
        glTexCoord2f(1,1); glVertex2f(x + surf->w, y);
        glTexCoord2f(1,0); glVertex2f(x + surf->w, y + surf->h);
        glTexCoord2f(0,0); glVertex2f(x, y + surf->h);
        glEnd();

        glDisable(GL_TEXTURE_2D);

        glPopMatrix(); // modelview
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        glDeleteTextures(1, &tex);
        SDL_FreeSurface(surf);
    }
#else
    // Fallback: set window title and print to stdout
    char title[256];
    snprintf(title, sizeof(title), "IARS - %s", buf);
    SDL_SetWindowTitle(window, title);
    // also print once per call to stdout for debugging
    printf("%s\n", buf);
#endif
}
