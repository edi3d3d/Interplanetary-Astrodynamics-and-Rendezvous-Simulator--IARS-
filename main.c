#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "camera.h"
#include "draw.h"
#include "planet.h"

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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

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

    glEnable(GL_DEPTH_TEST);
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
            v3_set(-0.763, -0.878, 0.457),              //v3_set(-401378746.763, -824301482.878, 18584913.457),
            v3_set(12.220388, 1.203173, -0.242563),
            1.989e30,
            696340.0
        ),
        create_planet(     // mercury
            v3_set(-13124187376.915, 44131704035.858, 4859427670.905),
            v3_set(-56637.275853, -11474.976199, 4257.660482),
            3.285e23,
            2439.7
        ),
        create_planet(     // venus
            v3_set(107983085979.426, 3996020448.719, -6168923518.074),
            v3_set(-1688.900947, 34827.313625, 576.373455),
            4.867e24,
            6051.8
        ),
        create_planet(     // earth
            v3_set(-134589859317.681, 61696396595.357, 15173720.840),
            v3_set(-13042.240188, -27118.517266, 1.246774),
            5.972e24,
            6371.0
        ),
        create_planet(     // mars
            v3_set(151179408197.375, -142541434528.848, -6668157338.425),
            v3_set(17480.684775, 19776.342928, -14.176741),
            6.417e23,
            3389.5
        ),
        create_planet(     // jupiter
            v3_set(-311402810884.616, 717326771284.569, 3993581443.098),
            v3_set(-12139.991385, -4582.581316, 290.769910),
            1.898e27,
            69911.0
        ),
        create_planet(     // saturn
            v3_set(1417424173888.574, 82656894872.498, -57872866065.981),
            v3_set(-1093.819313, 9622.798499, -124.145966),
            5.683e26,
            58232.0
        ),
        create_planet(     // uranus
            v3_set(1449907357843.363, 2526897397546.380, -9399101941.723),
            v3_set(-5956.878697, 3071.778866, 88.556547),
            8.681e25,
            25362.0
        ),
        create_planet(     // neptune
            v3_set(4467671534891.410, 102308478062.708, -105068965806.801),
            v3_set(-160.993904, 5465.729507, -108.542112),
            1.024e26,
            24622.0
        ),
        create_planet(     // pluto
            v3_set(2897954412330.601, -4428565391903.326, -364378457214.715),
            v3_set(4691.407215, 1747.668071, -1562.327369),
            1.309e22,
            1188.300
        ),
        create_planet(              // test
            v3_set(0, 0, 0),
            v3_set(0, 0, 0),
            1e2,
            100
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
        if (dt <= 0.0f) dt = 0.001f;
        if (dt > 0.2f) dt = 0.2f; // clamp large deltas
        lastTicks = now;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                int w = e.window.data1;
                int h = e.window.data2;
                glViewport(0, 0, w, h);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluPerspective(60.0, (double)w / (double)h, 0.1, 1000.0);
                glMatrixMode(GL_MODELVIEW);
            }
            camera_handle_event(&cam, &e);
        }

        // update camera from keyboard each frame
        const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);
        camera_update(&cam, keyboardState, dt);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // simple projection setup in case window resize event didn't run yet
        int w, h;
        SDL_GetWindowSize(win, &w, &h);
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60.0, (double)w / (double)h, 0.1, 10000000.0);
        //gluPerspective(60.0, (double)w / (double)h, 1.0, 1000000.0); //TODO: change to this after the simulation starts working
        glMatrixMode(GL_MODELVIEW);

        camera_apply_view(&cam);


        glShadeModel(GL_SMOOTH);
        glEnable(GL_DEPTH_TEST);

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

        for(u_int64_t i = 0; i < sizeof(bodies)/sizeof(bodies[0]); i++){
            draw_sphere(bodies[i].position, bodies[i].radius, 16, 16, colors[i]); // scale up radius for visibility
        }
        
        draw_cube(v3_set(0, 0, 0), 10.0f, colors); // test cube at origin
        draw_cuboid(v3_set(3, 0, 0), 1e5, 1e5, 1e-5, colors); // test cuboid at origin to visualize camera orientation (should look like a flat square)

        // draw coordinates HUD (top-right)
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
