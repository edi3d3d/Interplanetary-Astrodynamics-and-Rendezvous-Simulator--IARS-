#include "camera.h"
#include <GL/gl.h>
#include <GL/glu.h>

void camera_init(Camera *cam)
{
    cam->target[0] = 0.0f;
    cam->target[1] = 0.0f;
    cam->target[2] = 0.0f;
    cam->yaw = 0.0f;
    cam->pitch = 0.2f; // a little above the horizon
    cam->distance = 5.0f;
    cam->dragging = 0;
    cam->lastMouseX = cam->lastMouseY = 0;
    cam->sensitivityX = 0.005f;
    cam->sensitivityY = 0.005f;
    cam->zoomSpeed = 0.1f;
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
            cam->yaw   -= dx * cam->sensitivityX;
            cam->pitch += dy * cam->sensitivityY;
            // clamp pitch to avoid flipping
            const float maxPitch = 1.49f; // ~85 degrees
            if (cam->pitch > maxPitch) cam->pitch = maxPitch;
            if (cam->pitch < -maxPitch) cam->pitch = -maxPitch;
            cam->lastMouseX = e->motion.x;
            cam->lastMouseY = e->motion.y;
        }
        break;
    case SDL_MOUSEWHEEL:
        if (e->wheel.y > 0) {
            cam->distance *= (1.0f - cam->zoomSpeed);
            if (cam->distance < 0.1f) cam->distance = 0.1f;
        } else if (e->wheel.y < 0) {
            cam->distance *= (1.0f + cam->zoomSpeed);
            if (cam->distance > 100.0f) cam->distance = 100.0f;
        }
        break;
    default:
        break;
    }
}

void camera_apply_view(const Camera *cam)
{
    if (!cam) return;
    float cp = cosf(cam->pitch);
    float sp = sinf(cam->pitch);
    float cy = cosf(cam->yaw);
    float sy = sinf(cam->yaw);

    float camX = cam->target[0] + cam->distance * cp * sinf(cam->yaw);
    float camY = cam->target[1] + cam->distance * sp;
    float camZ = cam->target[2] + cam->distance * cp * cosf(cam->yaw);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camX, camY, camZ,
              cam->target[0], cam->target[1], cam->target[2],
              0.0f, 1.0f, 0.0f);
}
