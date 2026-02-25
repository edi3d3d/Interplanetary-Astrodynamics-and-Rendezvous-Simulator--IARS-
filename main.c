#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "camera.h"
#include "draw.h"

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
        gluPerspective(60.0, (double)w / (double)h, 0.1, 1000.0);
        //gluPerspective(60.0, (double)w / (double)h, 1.0, 1000000.0); //TODO: change to this after the simulation starts working
        glMatrixMode(GL_MODELVIEW);

        camera_apply_view(&cam);


        glShadeModel(GL_SMOOTH);
        glEnable(GL_DEPTH_TEST);

        Vec3 colors[6] = {
            v3_set(1,1,1), // bottom
            v3_set(1,1,0), // top
            v3_set(0,1,0), // front
            v3_set(1,0,0), // back
            v3_set(0,1,1), // left
            v3_set(0,0,1)  // right
        };

        Vec3 colors1[6] = {
            v3_set(0,1,1), // left
            v3_set(1,1,1), // front
            v3_set(1,0,0), // bottom
            v3_set(1,1,1), // top
            v3_set(1,1,1), // back
            v3_set(0,0,1)  // right
        };

        draw_cube(v3_set(0, 0, sqrt(2)), 1, colors);
        draw_cuboid(v3_set(0, 0, 0), 1e5, 1e5, 0.1f, colors1);

        //draw_sphere(v3_set(3, 0, 0), 0.5f, 16, 16, v3_set(1, 1, 1));

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


//todo: also implement local camera coordonates instead of world-space movement, so that moving forward always moves along the camera's forward axis, etc. This will require computing the camera's local axes (forward, right, up) from its orientation and using those to update position based on input.

//todo: add a simple 3D scene with some objects to navigate around, instead of just the single cube at the origin. This will help test the camera controls and make it more visually interesting. For example, add a grid floor and some cubes or spheres scattered around the scene.
//todo: (make the bodies x times larger to be more visible at the start, then scale them to the real size) add a database of celestial bodies (planets, moons, asteroids) with their orbital parameters, and render them in the scene with accurate positions based on the current time. This will allow testing the camera controls in a more realistic space environment and provide a visual reference for navigation.
//todo: add a simple UI overlay to display a list of the celestial bodies currently in view, along with their distances from the camera. This will help with navigation and provide useful information about the scene. The UI can be implemented using SDL_ttf to render text as textures in OpenGL.
//todo: add the realistic physics simulation of orbital mechanics, so that the celestial bodies move according to their orbits. This will allow testing the camera controls in a dynamic environment and provide a more immersive experience. The physics can be implemented using a simple numerical integrator (e.g. Euler or Verlet) to update the positions and velocities of the bodies each frame based on their gravitational interactions.
//todo: add a step mode to the simulation, allowing the user to pause and step through the simulation one frame at a time. This will help with debugging and allow for more detailed observation of the orbital mechanics in action. The step mode can be toggled with a key press (e.g. spacebar) and will pause the simulation until the user presses a key to advance to the next frame.
