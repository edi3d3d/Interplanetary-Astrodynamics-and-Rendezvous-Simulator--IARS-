#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "camera.h"
#include "draw.h"
#include "planet.h"
#include "render_utils.h"

// Load a reversed-Z perspective matrix into the current GL_PROJECTION matrix.
// This maps far->0 and near->1, allowing greater precision for large far planes
// when used with a floating-point depth buffer and glDepthFunc(GL_GREATER).
static void load_reversed_perspective(float fovy_deg, float aspect, float znear, float zfar)
{
    float fovy_rad = fovy_deg * (M_PI / 180.0);
    float f = 1.0 / tan(fovy_rad * 0.5);
    float m[16] = {0.0f};
    m[0] = (float)(f / aspect);
    m[5] = (float)f;
    // reversed-Z projection terms
    m[10] = (float)(znear / (znear - zfar));
    m[11] = -1.0f;
    m[14] = (float)((znear * zfar) / (znear - zfar));
    m[15] = 0.0f;
    glLoadMatrixf(m);
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    if(TTF_Init()) {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("DejaVuSans.ttf", 16);
    if (!font) {
        printf("Font load error: %s\n", TTF_GetError());
        return 1;
    }

    // Request a 24-bit depth buffer (or 32 if your system supports it).
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window *win = SDL_CreateWindow("3D Camera Test",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       1820, 980,
                                       SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glctx = SDL_GL_CreateContext(win);
    if (!glctx) {
        fprintf(stderr, "SDL_GL_CreateContext Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    // Use standard depth (clear to 1.0, depth test GL_LEQUAL) for now
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);

    glEnable(GL_CULL_FACE);
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);

    enum body{
        SUN,
        MERCURY,
        VENUS,
        EARTH,
        MARS,
        JUPITER,
        SATURN,
        URANUS,
        NEPTUNE,
        PLUTO,
        TEST
    };

    Planet bodies[] = {
        create_planet(              // sun
            v3_set(-0.763, -0.878, 0.457),      // real value is: v3_set(-401378746.763, -824301482.878, 18584913.457),
            v3_set(12.220388, 1.203173, -0.242563),
            1.989e30,
            696340.0
        ),
        create_planet(     // mercury
            v3_set(-13124187.376915, 44131704.035858, 4859427.670905),
            v3_set(-56.637275853, -11.474976199, 4.257660482),
            3.285e23,
            2439.7
        ),
        create_planet(     // venus
            v3_set(107983085.979426, 3996020.448719, -6168923.518074),
            v3_set(-1.688900947, 34.827313625, 0.576373455),
            4.867e24,
            6051.8
        ),
        create_planet(     // earth
            v3_set(-134589859.317681, 61696396.595357, 15173.720840),
            v3_set(-13.042240188, -27.118517266, 0.001246774),
            5.972e24,
            6371.0
        ),
        create_planet(     // mars
            v3_set(151179408.197375, -142541434.528848, -6668157.338425),
            v3_set(17.480684775, 19.776342928, -0.14176741),
            6.417e23,
            3389.5
        ),
        create_planet(     // jupiter
            v3_set(-311402810.884616, 717326771.284569, 3993581.443098),
            v3_set(-12.139991385, -4.582581316, 0.290769910),
            1.898e27,
            69911.0
        ),
        create_planet(     // saturn
            v3_set(1417424173.888574, 82656894.872498, -57872866.065981),
            v3_set(-1.093819313, 9.622798499, -0.124145966),
            5.683e26,
            58232.0
        ),
        create_planet(     // uranus
            v3_set(1449907357.84363, 2526897397.54638, -939910194.1723),
            v3_set(-5.956878697, 3.071778866, 0.088556547),
            8.681e25,
            25362.0
        ),
        create_planet(     // neptune
            v3_set(4467671534.891410, 1023084780.62708, -1050689658.06801),
            v3_set(-0.160993904, 5.465729507, -0.108542112),
            1.024e26,
            24622.0
        ),
        create_planet(     // pluto
            v3_set(2897954412.330601, -4428565391.903326, -364378457.214715),
            v3_set(4.691407215, 1.747668071, -1.562327369),
            1.309e22,
            1188.300
        ),
        create_planet(              // test
            v3_set(0.0, 0.0, 0.0),
            v3_set(0.0, 0.0, 0.0),
            1e2,
            100.0
        )
    };

    Camera cam;
    camera_init(&cam);

    Uint32 lastTicks = SDL_GetTicks();
    int running = 1;
    SDL_Event e;
    while (running) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - lastTicks) / 1000.0f;
        //if (dt <= 0.0f) dt = 0.001f;
        //if (dt > 0.2f) dt = 0.02f; // clamp large deltas
        lastTicks = now;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                int w = e.window.data1;
                int h = e.window.data2;
                glViewport(0, 0, w, h);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                // revert to standard perspective here for debugging
                gluPerspective(60.0, (float)w / (float)h, 0.1, 1e12);
                glMatrixMode(GL_MODELVIEW);
            }
            camera_handle_event(&cam, &e);
        }

        // update camera from keyboard each frame
        const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);
        camera_update(&cam, keyboardState, dt);

        // Simple per-frame integration: advance bodies by their velocity (position += velocity * dt)
        for (size_t i = 0; i < sizeof(bodies)/sizeof(bodies[0]); ++i) {
            bodies[i].position.x += bodies[i].velocity.x * dt;
            bodies[i].position.y += bodies[i].velocity.y * dt;
            bodies[i].position.z += bodies[i].velocity.z * dt;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // simple projection setup in case window resize event didn't run yet
        int w, h;
        SDL_GetWindowSize(win, &w, &h);
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        // use standard projection with a very large far plane
        gluPerspective(60.0, (float)w / (float)h, 0.1, 1e12);
        glMatrixMode(GL_MODELVIEW);

        camera_apply_view(&cam);

        // Ensure depth test and depth writes are enabled and configured for standard Z
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL); // standard Z
        glClearDepth(1.0);
        glFrontFace(GL_CCW);
        glEnable(GL_CULL_FACE);

        glShadeModel(GL_SMOOTH);

        Vec3 colors[] = {
            v3_set(1.0, 0.95, 0.80),     // Sun – warm white/yellow
            v3_set(0.60, 0.58, 0.55),    // Mercury – gray rocky
            v3_set(0.90, 0.80, 0.55),    // Venus – pale yellow/cream
            v3_set(0.20, 0.40, 0.90),    // Earth – deep ocean blue
            v3_set(0.75, 0.30, 0.15),    // Mars – rusty red
            v3_set(0.85, 0.70, 0.50),    // Jupiter – beige/orange bands
            v3_set(0.90, 0.82, 0.65),    // Saturn – pale golden
            v3_set(0.60, 0.85, 0.90),    // Uranus – light cyan
            v3_set(0.25, 0.45, 0.90),    // Neptune – deep blue
            v3_set(0.70, 0.65, 0.60),    // Pluto – light brown/gray
            v3_set(1.0, 1.0, 1.0)        // test – white
        };
        
        int body_count = sizeof(bodies)/sizeof(bodies[0]);
        
        //floatingOrigin(&(cam.position), bodies, body_count, 1e3);

        for(int i = 0; i < body_count; i++)
            draw_sphere(bodies[i].position, (float)bodies[i].radius, 64, 64, colors[i]);




        camera_draw_coordinates(&cam, win, font);

        SDL_GL_SwapWindow(win);

    }

    TTF_CloseFont(font);
    TTF_Quit();
    
    SDL_GL_DeleteContext(glctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}



//TODO: https://chatgpt.com/c/699ed4a2-1d30-838e-b831-6fb78931a67b, the new coordonate system for camera and planets
//*todo: also implement local camera coordonates instead of world-space movement, so that moving forward always moves along the camera's forward axis, etc. This will require computing the camera's local axes (forward, right, up) from its orientation and using those to update position based on input.

//todo: add a simple 3D scene with some objects to navigate around, instead of just the single cube at the origin. This will help test the camera controls and make it more visually interesting. For example, add a grid floor and some cubes or spheres scattered around the scene.
//todo: (make the bodies x times larger to be more visible at the start, then scale them to the real size) add a database of celestial bodies (planets, moons, asteroids) with their orbital parameters, and render them in the scene with accurate positions based on the current time. This will allow testing the camera controls in a more realistic space environment and provide a visual reference for navigation.
//todo: add a simple UI overlay to display a list of the celestial bodies currently in view, along with their distances from the camera. This will help with navigation and provide useful information about the scene. The UI can be implemented using SDL_ttf to render text as textures in OpenGL.
//todo: add the realistic physics simulation of orbital mechanics, so that the celestial bodies move according to their orbits. This will allow testing the camera controls in a dynamic environment and provide a more immersive experience. The physics can be implemented using a simple numerical integrator (e.g. Euler or Verlet) to update the positions and velocities of the bodies each frame based on their gravitational interactions.
//todo: add a step mode to the simulation, allowing the user to pause and step through the simulation one frame at a time. This will help with debugging and allow for more detailed observation of the orbital mechanics in action. The step mode can be toggled with a key press (e.g. spacebar) and will pause the simulation until the user presses a key to advance to the next frame.
