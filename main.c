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

#define DISTANCE_UNIT 1

#define SIZE_UNIT 1
#define MASS_UNIT 1

#define VELOCITY_UNIT 1


// Load a reversed-Z perspective matrix into the current GL_PROJECTION matrix.
// This maps far->0 and near->1, allowing greater precision for large far planes
// when used with a floating-point depth buffer and glDepthFunc(GL_GREATER).
// Load a reversed-Z perspective into GL_PROJECTION
static void load_reversed_perspective(float fovy_deg, float aspect, float znear, float zfar)
{
    const float fovy_rad = fovy_deg * (M_PI / 180.0f);
    const float f = 1.0f / tanf(fovy_rad * 0.5f);

    /* Column-major matrix as expected by glLoadMatrixf */
    float m[16] = {0.0f};
    m[0]  = f / aspect;
    m[5]  = f;
    /* reversed-Z: maps far -> 0, near -> 1
       Note: znear must be > 0. */
    m[10] = znear / (znear - zfar);
    m[11] = -1.0f;
    m[14] = (znear * zfar) / (znear - zfar);
    m[15] = 0.0f;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(m);
    glMatrixMode(GL_MODELVIEW);
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
    // Request depth size before creating context
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

    // Setup GL state after creating the context
    glEnable(GL_DEPTH_TEST);
    // Use standard depth (closest = less) here; keep consistent with gluPerspective usage
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0f);
    glDepthMask(GL_TRUE);
    
    glEnable(GL_CULL_FACE);
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    
    Planet bodies[] = {
        create_planet(          // Sun
            v3_set(-401378746.763263583183 * DISTANCE_UNIT, -824301482.877707958221 * DISTANCE_UNIT, 18584913.457012210041 * DISTANCE_UNIT),
            v3_set(12.220387675628 * VELOCITY_UNIT, 1.203172840033 * VELOCITY_UNIT, -0.242563321028 * VELOCITY_UNIT),
            1.98841e+30 * MASS_UNIT,
            348250000.0 * SIZE_UNIT
        ),

        create_planet(          // Mercury
            v3_set(-13124187376.914699554443 * DISTANCE_UNIT, 44131704035.857925415039 * DISTANCE_UNIT, 4859427670.905398368835 * DISTANCE_UNIT),
            v3_set(-56637.275852667495 * VELOCITY_UNIT, -11474.976198730210 * VELOCITY_UNIT, 4257.660482477958 * VELOCITY_UNIT),
            3.302e+23 * MASS_UNIT,
            1220265.0 * SIZE_UNIT
        ),

        create_planet(          // Venus
            v3_set(107983085979.426208496094 * DISTANCE_UNIT, 3996020448.719231128693 * DISTANCE_UNIT, -6168923518.073746681213 * DISTANCE_UNIT),
            v3_set(-1688.900947375086 * VELOCITY_UNIT, 34827.313624638038 * VELOCITY_UNIT, 576.373455333890 * VELOCITY_UNIT),
            4.8685e+24 * MASS_UNIT,
            3025946.50 * SIZE_UNIT
        ),

        create_planet(          // Earth
            v3_set(-134589859317.681106567383 * DISTANCE_UNIT, 61696396595.356620788574 * DISTANCE_UNIT, 15173720.839854329824 * DISTANCE_UNIT),
            v3_set(-13042.240188447369 * VELOCITY_UNIT, -27118.517266343813 * VELOCITY_UNIT, 1.246773782363 * VELOCITY_UNIT),
            5.97219e+24 * MASS_UNIT,
            492.82370 * SIZE_UNIT
        ),

        create_planet(          // Mars
            v3_set(151179408197.374694824219 * DISTANCE_UNIT, -142541434528.847900390625 * DISTANCE_UNIT, -6668157338.424719810486 * DISTANCE_UNIT),
            v3_set(17480.684774819219 * VELOCITY_UNIT, 19776.342927945181 * VELOCITY_UNIT, -14.176740913331 * VELOCITY_UNIT),
            6.41710e+23 * MASS_UNIT,
            1855.0 * SIZE_UNIT
        ),

        create_planet(          // Jupiter
            v3_set(-311402810884.615783691406 * DISTANCE_UNIT, 717326771284.569091796875 * DISTANCE_UNIT, 3993581443.098365783691 * DISTANCE_UNIT),
            v3_set(-12139.991384521780 * VELOCITY_UNIT, -4582.581316420221 * VELOCITY_UNIT, 290.769909651535 * VELOCITY_UNIT),
            1.89819e+27 * MASS_UNIT,
            34955500.0 * SIZE_UNIT
        ),

        create_planet(          // Saturn
            v3_set(1417424173888.573974609375 * DISTANCE_UNIT, 82656894872.497604370117 * DISTANCE_UNIT, -57872866065.980880737305 * DISTANCE_UNIT),
            v3_set(-1093.819312532118 * VELOCITY_UNIT, 9622.798499016606 * VELOCITY_UNIT, -124.145966098733 * VELOCITY_UNIT),
            5.6834e+26 * MASS_UNIT,
            29116000.0 * SIZE_UNIT
        ),

        create_planet(          // Uranus
            v3_set(1449907357843.363037109375 * DISTANCE_UNIT, 2526897397546.3798828125 * DISTANCE_UNIT, -9399101941.723464965820 * DISTANCE_UNIT),
            v3_set(-5956.878696625115 * VELOCITY_UNIT, 3071.778866025167 * VELOCITY_UNIT, 88.556546636943 * VELOCITY_UNIT),
            8.6813e+25 * MASS_UNIT,
            12681000.0 * SIZE_UNIT
        ),

        create_planet(          // Neptune
            v3_set(4467671534891.41015625 * DISTANCE_UNIT, 102308478062.707595825195 * DISTANCE_UNIT, -105068965806.800689697266 * DISTANCE_UNIT),
            v3_set(-160.993904220706 * VELOCITY_UNIT, 5465.729506548466 * VELOCITY_UNIT, -108.542111613209 * VELOCITY_UNIT),
            1.02409e+26 * MASS_UNIT,
            12312000.0 * SIZE_UNIT
        ),

        create_planet(          // Pluto
            v3_set(2897954412330.601074218750 * DISTANCE_UNIT, -4428565391903.326171875 * DISTANCE_UNIT, -364378457214.714538574219 * DISTANCE_UNIT),
            v3_set(4691.407215387665 * VELOCITY_UNIT, 1747.668070666197 * VELOCITY_UNIT, -1562.327369456416 * VELOCITY_UNIT),
            1.307e+22 * MASS_UNIT,
            594150.0 * SIZE_UNIT
        )
    };
    /*
    Planet bodies[] = {
        create_planet(              // test1
            v3_set(0, 0, 0),
            v3_set(1.0, 1.0, 1.1),
            1e2,
            1.0
        ),
        create_planet(              // test2
            v3_set(10.0, 10.0, 10.0),
            v3_set(0.0, 0.0, 0.0),
            1e10,
            1.0
        )
    };
    */

    float startingEnergy = systemEnergy(bodies, sizeof(bodies)/sizeof(bodies[0]));

    Camera cam;
    camera_init(&cam);

    /* Time warp: multiply physics timestep by this factor.
       Controls: '+' or keypad '+' doubles warp, '-' halves warp, '0' resets to 1x. */
    float time_warp = 1.0f;
    const float time_warp_min = 1e-6f;
    const float time_warp_max = 1e8f;

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
                // in resize handler:
                load_reversed_perspective(60.0f, (float)w / (float)h, 0.1f, 1e12f);
                glMatrixMode(GL_MODELVIEW);
            }
            else if (e.type == SDL_KEYDOWN) {
                SDL_Keycode k = e.key.keysym.sym;
                if (k == SDLK_EQUALS || k == SDLK_PLUS || k == SDLK_KP_PLUS) {
                    time_warp *= 2.0f;
                    if (time_warp > time_warp_max) time_warp = time_warp_max;
                    printf("Time warp: %.0fx\n", time_warp);
                    continue;
                } else if (k == SDLK_MINUS || k == SDLK_KP_MINUS) {
                    time_warp *= 0.5f;
                    if (time_warp < time_warp_min) time_warp = time_warp_min;
                    printf("Time warp: %.0fx\n", time_warp);
                    continue;
                } else if (k == SDLK_0 || k == SDLK_KP_0) {
                    time_warp = 1.0f;
                    printf("Time warp reset to 1x\n");
                    continue;
                }
            }
             camera_handle_event(&cam, &e);
         }

        // update camera from keyboard each frame
        const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);
        camera_update(&cam, keyboardState, dt);

        int bodyCount = sizeof(bodies)/sizeof(bodies[0]);

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
        floatingOrigin_d(&cam, bodies, bodyCount, 1e5 /* 100k km threshold */);

        float sim_dt = dt * time_warp;
        planetGravityUpdate(bodies, bodyCount, sim_dt);

        for(int i = 0; i < bodyCount; i++)
            //draw_cube(bodies[i].position, (float)bodies[i].radius, NULL);
            //draw_sphere(bodies[i].position, (float)bodies[i].radius, 64, 64, colors[i]);
            draw_planet_or_indicator(bodies[i], &cam, w, h, 64, 64, colors[i], 20.0f);


        float currentEnergy = systemEnergy(bodies, bodyCount);
        float energyDifference = currentEnergy - startingEnergy;
        printf("Current Energy: %f, Energy Difference: %f\n", currentEnergy, energyDifference);

        camera_draw_coordinates(&cam, win, font, time_warp);
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

//todo: (add a way to see where all planets are) add a database of celestial bodies (planets, moons, asteroids) with their orbital parameters, and render them in the scene with accurate positions based on the current time. This will allow testing the camera controls in a more realistic space environment and provide a visual reference for navigation.
//todo: add a simple UI overlay to display a list of the celestial bodies currently in view, along with their distances from the camera. This will help with navigation and provide useful information about the scene. The UI can be implemented using SDL_ttf to render text as textures in OpenGL.
//todo: add the realistic physics simulation of orbital mechanics, so that the celestial bodies move according to their orbits. This will allow testing the camera controls in a dynamic environment and provide a more immersive experience. The physics can be implemented using a simple numerical integrator (e.g. Euler or Verlet) to update the positions and velocities of the bodies each frame based on their gravitational interactions.
//todo: add a step mode to the simulation, allowing the user to pause and step through the simulation one frame at a time. This will help with debugging and allow for more detailed observation of the orbital mechanics in action. The step mode can be toggled with a key press (e.g. spacebar) and will pause the simulation until the user presses a key to advance to the next frame.