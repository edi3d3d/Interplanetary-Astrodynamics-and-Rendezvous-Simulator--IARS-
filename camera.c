#include <stdio.h>
#include <math.h>

#include <SDL2/SDL.h>

#include <GL/glu.h>
#include <GL/gl.h>

#include "camera.h"
#include "vec3.h"
#include "quaternions.h"

static const float DEG2RAD = 0.017453292519943295769236907684886f;

static void camera_begin_speed_input(Camera *cam)
{
    cam->speedInputActive = 1;
    cam->speedInputLen = 0;
    cam->speedInputBuf[0] = '\0';
    SDL_StartTextInput();
}

static void camera_cancel_speed_input(Camera *cam)
{
    cam->speedInputActive = 0;
    cam->speedInputLen = 0;
    cam->speedInputBuf[0] = '\0';
    SDL_StopTextInput();
}

static void camera_commit_speed_input(Camera *cam)
{
    if (cam->speedInputLen == 0) {
        camera_cancel_speed_input(cam);
        return;
    }

    char *endp = NULL;
    float v = strtof(cam->speedInputBuf, &endp);

    // Accept only if parsed something and it's finite and > 0
    if (endp != cam->speedInputBuf && isfinite(v) && v > 0.0f) {
        cam->moveSpeed = v;
    }

    camera_cancel_speed_input(cam);
}

static int is_allowed_speed_char(char c)
{
    return (c >= '0' && c <= '9') || c == '.' || c == '-';
}

static void camera_apply_yaw_pitch(Camera *cam, float yawDeltaRad, float pitchDeltaRad)
{
    // local axes: forward +X, side +Y, up +Z
    Vec3 localUp   = v3_set(0.0f, 0.0f, 1.0f);
    Vec3 localSide = v3_set(0.0f, 1.0f, 0.0f);

    // yaw around camera-up (in world)
    Vec3 yawAxisWorld = q_rotate_vec3(cam->rot, localUp);
    yawAxisWorld = v3_normalize(yawAxisWorld);

    Quaternion qYaw = q_from_axis_angle(yawAxisWorld, yawDeltaRad);
    cam->rot = q_mul(qYaw, cam->rot);
    cam->rot = q_normalize(cam->rot);

    // pitch around camera-side (after yaw)
    Vec3 pitchAxisWorld = q_rotate_vec3(cam->rot, localSide);
    pitchAxisWorld = v3_normalize(pitchAxisWorld);

    Quaternion qPitch = q_from_axis_angle(pitchAxisWorld, pitchDeltaRad);
    cam->rot = q_mul(qPitch, cam->rot);
    cam->rot = q_normalize(cam->rot);
}

void camera_init(Camera *cam)
{
    if (!cam) return;

    cam->position = v3_set(-13124187.0f, 44131704.0f, 4859427.0f);
    cam->position = v3_set(0.0f, 0.0f, 10.0f); // debug
    cam->rot = q_set(1.0f, 0.0f, 0.0f, 0.0f);

    cam->position_d = v3_set(cam->position.x, cam->position.y, cam->position.z); // initialize double-precision position to the same value

    cam->moveSpeed = 5.0f;

    cam->sensitivityX = 0.15f;
    cam->sensitivityY = 0.15f;

    cam->mouseRotateSpeed = 1.0f;
    cam->zoomSpeed = 0.1f;

    cam->dragging = 0;
    cam->lastMouseX = cam->lastMouseY = 0;

    cam->lookLeft = cam->lookRight = cam->lookUp = cam->lookDown = 0;
    cam->keyLookSpeedDeg = 90.0f;

    cam->speedInputActive = 0;
    cam->speedInputLen = 0;
    cam->speedInputBuf[0] = '\0';
}

void camera_handle_event(Camera *cam, const SDL_Event *e)
{
    if (!cam || !e) return;

    if (cam->speedInputActive) {
        if (e->type == SDL_TEXTINPUT) {
            for (const char *p = e->text.text; *p; ++p) {
                if (!is_allowed_speed_char(*p)) continue;
                if (cam->speedInputLen < (int)sizeof(cam->speedInputBuf) - 1) {
                    cam->speedInputBuf[cam->speedInputLen++] = *p;
                    cam->speedInputBuf[cam->speedInputLen] = '\0';
                }
            }
            return;
        }

        if (e->type == SDL_KEYDOWN) {
            SDL_Keycode k = e->key.keysym.sym;

            if (k == SDLK_BACKSPACE) {
                if (cam->speedInputLen > 0) {
                    cam->speedInputBuf[--cam->speedInputLen] = '\0';
                }
                return;
            }
            if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
                camera_commit_speed_input(cam);
                return;
            }
            if (k == SDLK_ESCAPE) {
                camera_cancel_speed_input(cam);
                return;
            }
        }

        // While typing, ignore other camera controls
        return;
    }

    // --- normal camera events ---
    if (e->type == SDL_KEYDOWN) {
        // Press I to start typing speed
        if (e->key.keysym.sym == SDLK_i) {
            camera_begin_speed_input(cam);
            return;
        }
    }

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

        float yawDeltaRad   = (-dx) * cam->sensitivityX * cam->mouseRotateSpeed * DEG2RAD;
        float pitchDeltaRad = ( dy) * cam->sensitivityY * cam->mouseRotateSpeed * DEG2RAD;

        camera_apply_yaw_pitch(cam, yawDeltaRad, pitchDeltaRad);
        return;
    }

    if (e->type == SDL_KEYDOWN || e->type == SDL_KEYUP) {
        int down = (e->type == SDL_KEYDOWN);

        // optional: ignore key repeat so holding doesn't spam events (we use held-state anyway)
        // if (e->type == SDL_KEYDOWN && e->key.repeat) return;

        switch (e->key.keysym.sym) {
            case SDLK_LEFT:  cam->lookLeft  = down; break;
            case SDLK_RIGHT: cam->lookRight = down; break;
            case SDLK_UP:    cam->lookUp    = down; break;
            case SDLK_DOWN:  cam->lookDown  = down; break;
            default: break;
        }
    }
}

void camera_update(Camera *cam, const Uint8 *keyboardState, float dt)
{
    if (!cam || !keyboardState) return;

    if (cam->speedInputActive) {
        // No movement/rotation while typing
        return;
    }

    int dx = (cam->lookRight ? 1 : 0) - (cam->lookLeft ? 1 : 0);
    int dy = (cam->lookDown  ? 1 : 0) - (cam->lookUp   ? 1 : 0);

    if (dx != 0 || dy != 0) {
        float yawDeltaRad   = (-(float)dx) * cam->keyLookSpeedDeg * DEG2RAD * dt;
        float pitchDeltaRad = ( (float)dy) * cam->keyLookSpeedDeg * DEG2RAD * dt;

        camera_apply_yaw_pitch(cam, yawDeltaRad, pitchDeltaRad);
    }

    Vec3 movementLocal = v3_set(0.0f, 0.0f, 0.0f);

    // movement
    if (keyboardState[SDL_SCANCODE_W]) movementLocal.x += 1.0f;
    if (keyboardState[SDL_SCANCODE_S]) movementLocal.x -= 1.0f;

    if (keyboardState[SDL_SCANCODE_A]) movementLocal.y += 1.0f;
    if (keyboardState[SDL_SCANCODE_D]) movementLocal.y -= 1.0f;

    if (keyboardState[SDL_SCANCODE_LSHIFT] || keyboardState[SDL_SCANCODE_RSHIFT]) movementLocal.z += 1.0f;
    if (keyboardState[SDL_SCANCODE_LCTRL]  || keyboardState[SDL_SCANCODE_RCTRL])  movementLocal.z -= 1.0f;

    // roll (Q/E)
    int rollDir = 0;
    if (keyboardState[SDL_SCANCODE_E]) rollDir += 1;
    if (keyboardState[SDL_SCANCODE_Q]) rollDir -= 1;

    // roll
    if (rollDir != 0) {
        const float rollSpeedDeg = 90.0f;
        float rollDeltaRad = (rollDir * rollSpeedDeg) * dt * DEG2RAD;

        Vec3 localForward = v3_set(1.0f, 0.0f, 0.0f);
        Vec3 rollAxisWorld = q_rotate_vec3(cam->rot, localForward);
        rollAxisWorld = v3_normalize(rollAxisWorld);

        Quaternion qRoll = q_from_axis_angle(rollAxisWorld, rollDeltaRad);
        cam->rot = q_mul(qRoll, cam->rot);
        cam->rot = q_normalize(cam->rot);
    }
    // movement: convert local movement to world (float), then to float and apply to position_d
    if (!v3_is_zero(movementLocal)) {
        movementLocal = v3_normalize(movementLocal);
        
        Vec3 movementWorld = q_rotate_vec3(cam->rot, movementLocal);


        float scale = (float)cam->moveSpeed * (float)dt;

        cam->position = v3_add(cam->position, v3_scale(movementWorld, scale));
        cam->position_d = v3_add(cam->position_d, v3_scale(movementWorld, scale));


        //from now on new method

    }
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

    // center point = pos + forward (local forward, can be made larger but it doesnt matter that much >= 0)
    Vec3 center = v3_add(pos, forward);

    gluLookAt(pos.x, pos.y, pos.z,
              center.x, center.y, center.z,
              up.x, up.y, up.z);
}

void draw_surface(SDL_Surface *surf, int drawX, int drawY)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, surf->pitch / 4);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 surf->w, surf->h, 0,
                 GL_BGRA, GL_UNSIGNED_BYTE,
                 surf->pixels);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glColor4f(1.f, 1.f, 1.f, 1.f);

    glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex2f(drawX, drawY);
        glTexCoord2f(1,0); glVertex2f(drawX+surf->w, drawY);
        glTexCoord2f(1,1); glVertex2f(drawX+surf->w, drawY+surf->h);
        glTexCoord2f(0,1); glVertex2f(drawX, drawY+surf->h);
    glEnd();

    glDeleteTextures(1, &tex);
};

void camera_draw_coordinates(const Camera *cam, SDL_Window *window, TTF_Font *font)
{
    if (!cam || !window || !font) return;

    char line1[128];
    char line2[128];
    char line3[128];
    char line4[128];

    snprintf(line1, sizeof(line1),
             "Global Pos: %.2f, %.2f, %.2f",
             cam->position_d.x, cam->position_d.y, cam->position_d.z);

    snprintf(line2, sizeof(line2),
             "Pos: %.2f, %.2f, %.2f",
             cam->position.x, cam->position.y, cam->position.z);

    snprintf(line3, sizeof(line3),
             "Q(w,x,y,z): %.3f, %.3f, %.3f, %.3f",
             cam->rot.w, cam->rot.v.x, cam->rot.v.y, cam->rot.v.z);

    int show4 = 0;
    if (cam->speedInputActive) {
        snprintf(line4, sizeof(line4), "Set moveSpeed: %s", cam->speedInputBuf);
        show4 = 1;
    } else {
        // optional: show current speed when not typing
        snprintf(line4, sizeof(line4), "moveSpeed: %.3f", cam->moveSpeed);
        show4 = 1;
    }

    SDL_Color black = {0, 0, 0, 255};

    SDL_Surface *s1 = TTF_RenderUTF8_Blended(font, line1, black);
    SDL_Surface *s2 = TTF_RenderUTF8_Blended(font, line2, black);
    SDL_Surface *s3 = TTF_RenderUTF8_Blended(font, line3, black);
    SDL_Surface *s4 = show4 ? TTF_RenderUTF8_Blended(font, line4, black) : NULL;

    if (!s1 || !s2 || !s3 || (show4 && !s4)) {
        if (s1) SDL_FreeSurface(s1);
        if (s2) SDL_FreeSurface(s2);
        if (s3) SDL_FreeSurface(s3);
        if (s4) SDL_FreeSurface(s4);
        return;
    }

    SDL_Surface *c1 = SDL_ConvertSurfaceFormat(s1, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_Surface *c2 = SDL_ConvertSurfaceFormat(s2, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_Surface *c3 = SDL_ConvertSurfaceFormat(s3, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_Surface *c4 = show4 ? SDL_ConvertSurfaceFormat(s4, SDL_PIXELFORMAT_ARGB8888, 0) : NULL;

    SDL_FreeSurface(s1);
    SDL_FreeSurface(s2);
    SDL_FreeSurface(s3);
    if (s4) SDL_FreeSurface(s4);

    if (!c1 || !c2 || !c3 || (show4 && !c4)) {
        if (c1) SDL_FreeSurface(c1);
        if (c2) SDL_FreeSurface(c2);
        if (c3) SDL_FreeSurface(c3);
        if (c4) SDL_FreeSurface(c4);
        return;
    }

    int textW = c1->w;
    if (c2->w > textW) textW = c2->w;
    if (c3->w > textW) textW = c3->w;
    if (show4 && c4->w > textW) textW = c4->w;

    int textH = c1->h + c2->h + c3->h + (show4 ? c4->h : 0);

    int winW, winH;
    SDL_GetWindowSize(window, &winW, &winH);

    int pad = 6;
    int x = winW - textW - 10;
    int y = 10;

    int bgX = x - pad;
    int bgY = y - pad;
    int bgW = textW + pad * 2;
    int bgH = textH + pad * 2;

    // Save GL state
    GLboolean wasTex2D = glIsEnabled(GL_TEXTURE_2D);
    GLboolean wasCull  = glIsEnabled(GL_CULL_FACE);
    GLboolean wasDepth = glIsEnabled(GL_DEPTH_TEST);
    GLboolean wasBlend = glIsEnabled(GL_BLEND);
    GLint oldMatrixMode;
    glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, winW, winH, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // White background
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.f, 1.f, 1.f, 1.f);
    glBegin(GL_QUADS);
        glVertex2f(bgX, bgY);
        glVertex2f(bgX + bgW, bgY);
        glVertex2f(bgX + bgW, bgY + bgH);
        glVertex2f(bgX, bgY + bgH);
    glEnd();

    // Black border
    glColor4f(0.f, 0.f, 0.f, 1.f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(bgX, bgY);
        glVertex2f(bgX + bgW, bgY);
        glVertex2f(bgX + bgW, bgY + bgH);
        glVertex2f(bgX, bgY + bgH);
    glEnd();

    // Draw text lines
    draw_surface(c1, x, y);
    draw_surface(c2, x, y + c1->h);
    draw_surface(c3, x, y + c1->h + c2->h);
    if (show4) draw_surface(c4, x, y + c1->h + c2->h + c3->h);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(oldMatrixMode);

    if (wasDepth) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (wasCull)  glEnable(GL_CULL_FACE);  else glDisable(GL_CULL_FACE);
    if (wasBlend) glEnable(GL_BLEND);      else glDisable(GL_BLEND);
    if (wasTex2D) glEnable(GL_TEXTURE_2D); else glDisable(GL_TEXTURE_2D);

    SDL_FreeSurface(c1);
    SDL_FreeSurface(c2);
    SDL_FreeSurface(c3);
    if (c4) SDL_FreeSurface(c4);
}

