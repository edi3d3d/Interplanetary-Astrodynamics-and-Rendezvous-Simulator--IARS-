#ifndef CAMERA_H
#define CAMERA_H

#include <SDL2/SDL.h>
#include <math.h>

typedef struct Camera {
    float target[3];    // point camera orbits around
    float yaw;          // rotation around Y (radians)
    float pitch;        // rotation around X (radians)
    float distance;     // distance from target (zoom)

    int dragging;       // mouse left button dragging flag
    int lastMouseX;
    int lastMouseY;

    float sensitivityX; // rotation sensitivity
    float sensitivityY; // rotation sensitivity
    float zoomSpeed;
} Camera;

// Initialize camera with defaults
void camera_init(Camera *cam);

// Handle SDL events related to the camera (mouse motion, wheel, buttons)
void camera_handle_event(Camera *cam, const SDL_Event *e);

// Apply the camera transform (calls gluLookAt internally)
void camera_apply_view(const Camera *cam);

#endif // CAMERA_H
