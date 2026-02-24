#ifndef CAMERA_H
#define CAMERA_H

#include <SDL2/SDL.h>
#include <math.h>
#include "vec3.h"
#include "quaternions.h"

typedef struct Camera {
    // world-space camera position (computed from base + local)
    // (x, y, z) x is forward, y is left, z is up, weird
    //
    // x forward/back, y for left/right, z for up/down
    Vec3 position;

    // point the camera can orbit around (used as base in orbit mode)
    Vec3 target;

    // Euler angles in degrees packed in a Quaternion: (w, x, y, z)
    // These are the camera-local rotation values the UI and controls modify.
    Quaternion rot;

    // distance from target when in orbit mode (zoom)
    float distance;

    // mouse drag state
    int dragging;       // mouse left button dragging flag
    int lastMouseX;
    int lastMouseY;

    // movement/rotation parameters
    float moveSpeed;     // units per second for WSAD/vertical movement
    float sensitivityX;  // mouse X rotation sensitivity
    float sensitivityY;  // mouse Y rotation sensitivity
    float mouseRotateSpeed; // multiplier for mouse rotation when dragging
    float zoomSpeed;     // wheel zoom speed

    int freeMode;        // when set, camera uses position + orientation (6-DOF) instead of orbiting target

    // local-space offset (camera-local coordinates) and base (world-space anchor)
    // localPosition is in camera-local axes: X=right, Y=up, Z=forward (forward positive)
    // We store localPosition so input modifies camera-local coords; camera computes world position from basePosition + rotated(localPosition).
    Vec3 localPosition; // units in same units as world
    Vec3 basePosition;  // world-space anchor (typically equal to target when not in freeMode)
} Camera;

// Initialize camera with sensible defaults
void camera_init(Camera *cam);

// Handle SDL events related to the camera (mouse motion, wheel, buttons)
// - mouse wheel should zoom in/out (adjusts cam->distance or moves along forward)
// - mouse button press toggles dragging (holding left button + mouse move rotates yaw/pitch)
void camera_handle_event(Camera *cam, const SDL_Event *e);

// Update camera from keyboard state. Call once per frame with dt (seconds).
// Keyboard controls (expected SDL keyboard state):
// - W / S : move forward / backward along camera forward vector
// - A / D : strafe left / right along camera right vector
// - Left Shift : move up (camera-local up)
// - Left Ctrl  : move down (camera-local down)
// - Q / E : roll left / right (rotate around forward axis)
// Use SDL_GetKeyboardState(NULL) to get the keyboardState argument.
void camera_update(Camera *cam, const Uint8 *keyboardState, float dt);

// Apply the camera transform (e.g. using gluLookAt or a custom matrix)
void camera_apply_view(const Camera *cam);

// Draw camera coordinates in the top-right corner of the given SDL_Window.
// If SDL_ttf is available define USE_SDL_TTF at compile time and link with SDL2_ttf to render on-screen text.
// Otherwise the function will print coordinates to stdout as a fallback.
void camera_draw_coordinates(const Camera *cam, SDL_Window *window);

#endif // CAMERA_H