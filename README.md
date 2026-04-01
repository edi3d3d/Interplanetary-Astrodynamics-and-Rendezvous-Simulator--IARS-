# Interplanetary Astrodynamics and Rendezvous Simulator (IARS)

Small C-based simulator and visualizer for basic interplanetary dynamics and rendezvous testing.  
The project is a work-in-progress: it combines simple physics integration, a camera with large-world support, and an OpenGL-based renderer driven by SDL2.

## Key facts
- Language: C (C11)
- Rendering: OpenGL + GLU (immediate mode)
- Windowing / input / text: SDL2, SDL2_ttf
- Build: Makefile (gcc + sdl2-config)

## Requirements (development)
- GCC (or compatible C compiler) with C11 support
- SDL2 development headers & libs
- SDL2_ttf
- OpenGL / GLU development headers & libs
- make, pkg-config

On Debian/Ubuntu:
sudo apt install build-essential libsdl2-dev libsdl2-ttf-dev libglu1-mesa-dev libgl1-mesa-dev

## Build & run
make
./main
or
make run

The Makefile uses `sdl2-config` to obtain compiler/linker flags.

## Behavior & caveats
- The simulator currently stores positions and velocities in single-precision Vec3 (float). At very large coordinates this causes visible precision loss. The code contains partial support for double-precision camera position and a floating-origin recenter technique to mitigate this.
- Floating origin: when the camera drifts far from origin the world can be recentlyred by subtracting the camera delta from all bodies. This is an occasional O(N) operation and may cause a visible jump unless handled with hysteresis or smoothing.
- Depth: projection and depth-test mode must match (standard Z vs reversed‑Z). See code for a reversed‑Z loader if you want better far-plane precision.
- Camera: input currently applies immediate displacement. If you observe momentum/inertia verify camera_update and remove velocity accumulation.

## Development notes
- Prefer keeping physics in double precision (Vec3d) and only converting to float immediately before rendering to avoid float rounding issues.
- If you change the coordinate representation, update render_utils, camera, planet modules consistently.
- The code is actively being debugged; expect incomplete features and TODOs in source.

## Contributing
Open an issue or submit a pull request. Keep changes small and test both build and runtime behavior.

## ToDo:
- Implement the planetary motion for the orbits
- Implement a way to see where all planets and objects are at even when the planet in not visible
- Implement a way to teleport to a certain coordonate or a planet from a list
<br><br>
- Change the spheres to use less stacks/slices the further away they are
<br><br>
- Add small probes
- Maybe implement the rendezvous in the next 0-200 years
- More to be added
## License
MIT License

Copyright (c) 2026 Dinu Eduard-Alexandru