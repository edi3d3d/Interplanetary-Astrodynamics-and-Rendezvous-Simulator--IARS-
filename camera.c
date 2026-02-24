#include "camera.h"
#include "vec3.h"
#include "quaternions.h"
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <math.h>

static const float DEG2RAD = 0.017453292519943295769236907684886f;



void camera_init(Camera *cam)
{
    if (!cam) return;

    cam->position = v3_set(-5.0f, 0.0f, 0.0f);
    cam->rot = q_set(1.0f, 0.0f, 0.0f, 0.0f); // identity

    cam->moveSpeed = 5.0f;

    // Treat these as "degrees per pixel" (typical). We'll convert to radians.
    cam->sensitivityX = 0.15f;
    cam->sensitivityY = 0.15f;

    cam->mouseRotateSpeed = 1.0f;
    cam->zoomSpeed = 0.1f;

    cam->dragging = 0;
    cam->lastMouseX = cam->lastMouseY = 0;
}

void camera_handle_event(Camera *cam, const SDL_Event *e)
{
    if (!cam || !e) return;

    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        cam->dragging = 1;
        cam->lastMouseX = e->button.x;
        cam->lastMouseY = e->button.y;
        return;
    }

    if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT) {
        cam->dragging = 0;
        return;
    }

    if (e->type == SDL_MOUSEMOTION && cam->dragging) {
        int dx = e->motion.x - cam->lastMouseX;
        int dy = e->motion.y - cam->lastMouseY;

        cam->lastMouseX = e->motion.x;
        cam->lastMouseY = e->motion.y;

        // Convert mouse deltas to radians.
        // If sensitivity is degrees/pixel, multiply by DEG2RAD.
        float yawDeltaRad   = (-dx) * cam->sensitivityX * cam->mouseRotateSpeed * DEG2RAD;
        float pitchDeltaRad = (dy) * cam->sensitivityY * cam->mouseRotateSpeed * DEG2RAD;

        // Local basis at identity:
        // forward = +X, side = +Y, up = +Z
        Vec3 localUp   = v3_set(0.0f, 0.0f, 1.0f);
        Vec3 localSide = v3_set(0.0f, 1.0f, 0.0f);

        // --- YAW about camera's CURRENT up axis (not world up) ---
        Vec3 yawAxisWorld = q_rotate_vec3(cam->rot, localUp);
        yawAxisWorld = v3_normalize(yawAxisWorld);

        Quaternion qYaw = q_from_axis_angle(yawAxisWorld, yawDeltaRad);
        cam->rot = q_mul(qYaw, cam->rot);
        cam->rot = q_normalize(cam->rot);

        // --- PITCH about camera's CURRENT side axis ---
        Vec3 pitchAxisWorld = q_rotate_vec3(cam->rot, localSide);
        pitchAxisWorld = v3_normalize(pitchAxisWorld);

        Quaternion qPitch = q_from_axis_angle(pitchAxisWorld, pitchDeltaRad);
        cam->rot = q_mul(qPitch, cam->rot);
        cam->rot = q_normalize(cam->rot);
    }
}

void camera_update(Camera *cam, const Uint8 *keyboardState, float dt)
{
    if (!cam || !keyboardState) return;

    Vec3 movementLocal = v3_set(0.0f, 0.0f, 0.0f);

    // Local movement intent in camera coordinates:
    // X: along localForward (+X)
    // Y: along localSide (+Y)
    // Z: along localUp (+Z)
    if (keyboardState[SDL_SCANCODE_W]) movementLocal.x += 1.0f;
    if (keyboardState[SDL_SCANCODE_S]) movementLocal.x -= 1.0f;

    if (keyboardState[SDL_SCANCODE_A]) movementLocal.y += 1.0f;
    if (keyboardState[SDL_SCANCODE_D]) movementLocal.y -= 1.0f;

    if (keyboardState[SDL_SCANCODE_LSHIFT] || keyboardState[SDL_SCANCODE_RSHIFT]) movementLocal.z += 1.0f;
    if (keyboardState[SDL_SCANCODE_LCTRL]  || keyboardState[SDL_SCANCODE_RCTRL])  movementLocal.z -= 1.0f;

    if (v3_is_zero(movementLocal)) return;

    // Normalize so diagonal movement isn't faster
    movementLocal = v3_normalize(movementLocal);

    // Transform local movement into world movement using orientation
    Vec3 movementWorld = q_rotate_vec3(cam->rot, movementLocal);

    cam->position = v3_add(cam->position, v3_scale(movementWorld, cam->moveSpeed * dt));
}

void camera_apply_view(const Camera *cam)
{
    if (!cam) return;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    Vec3 pos = cam->position;

    // Derive camera axes from quaternion
    Vec3 forward = q_rotate_vec3(cam->rot, v3_set(1.0f, 0.0f, 0.0f)); // localForward -> world
    Vec3 up      = q_rotate_vec3(cam->rot, v3_set(0.0f, 0.0f, 1.0f)); // localUp      -> world

    forward = v3_normalize(forward);
    up      = v3_normalize(up);

    // Any positive distance works; 1 is enough
    Vec3 center = v3_add(pos, forward);

    gluLookAt(pos.x, pos.y, pos.z,
              center.x, center.y, center.z,
              up.x, up.y, up.z);
}

void camera_draw_coordinates(const Camera *cam, SDL_Window *window)
{
    if (!cam || !window) return;

    // Quaternion isn't (pitch,yaw,roll). Show position + quaternion components.
    char buf[160];
    snprintf(buf, sizeof(buf),
             "Pos: %.2f, %.2f, %.2f  Q(w,x,y,z): %.3f, %.3f, %.3f, %.3f",
             cam->position.x, cam->position.y, cam->position.z,
             cam->rot.w, cam->rot.v.x, cam->rot.v.y, cam->rot.v.z);

    SDL_SetWindowTitle(window, buf);
}