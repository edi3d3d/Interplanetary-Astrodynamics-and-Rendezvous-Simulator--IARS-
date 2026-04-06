# Interplanetary Astrodynamics and Rendezvous Simulator (IARS)
Interplanetary Astrodynamics and Rendezvous Simulator (IARS) is a compact C-based simulator + optimiser that helps find feasible, low‑Δv transfer routes between two states (planets, moons, or Cartesian states) under user constraints. It combines fast two‑body candidate generation with an N‑body integrator for verification.
<br><br>
For now, its a somewhat accurate simulator of the Solar system.

## Goal
- Accept a start and target (named body or Cartesian state, TBD).
- Generate candidate transfers meeting constraints (time‑of‑flight, max Δv, windows, optional gravity assists).
- Return ranked routes with Δv estimates, TOF, maneuver schedule, and N‑body verification.

## Quick CLI example TBD
```
./main --optimize --from "Probe1" --to "Probe2" --earliest "2028-01-01" --max-dv 9000 --max-acceleration 4g --fuel 1500 --isp 350
```

## What it should do (high level)
- Fast candidate generation using two‑body methods (Lambert, Hohmann, patch‑conics).
- Score and rank candidates (Δv baseline + penalties).
- Verify promising candidates with the N‑body integrator (apply impulses, simulate, measure error).
- Provide JSON/text output and simple visualization of candidate trajectories.

## Core C sources (short)
- `main.c` - entry point, init/shutdown, SDL/OpenGL setup, main loop, CLI parsing, time‑warp and orchestration of physics + rendering.
- `planet.c` - planet data structures and N‑body integrator (Velocity‑Verlet with substepping). Pairwise gravity and acceleration workspace are here; used by verification and simulation.
- `formulas.c` - physics helpers, constants and unit-safe conversions (gravity, GM<->mass conversions).
- `vec3.c` / `vec3.h` - vector primitives used throughout (add/sub/scale/dot/cross/len/normalize). Keep these small and efficient.
- `draw.c` / `draw.h` - rendering helpers, on/off‑screen indicators and HUD primitives for visual debugging.
- `render_utils.c` - floating‑origin recentering utilities; ensure recentering keeps simulation and rendering states consistent.
- `camera.c` / `quaternions.c` - camera input/state and quaternion helpers used for view transforms.

## Small note about fetching ephemerides
- Use `fetch.py` (or `fetch.c`) to query JPL Horizons (VECTORS + OBJ_DATA). Note: The fetchers do not work ATM, should be fixed at some point, parsing to be changed
- Key parsing points: quote START/STOP times, handle Fortran 'D' exponents, extract position (km) and velocity (km/s), and parse mass/GM/diameter if present.
- Convert Horizons outputs into the simulator units before import (respect `DISTANCE_UNIT`/`VELOCITY_UNIT` or convert to SI). Prefer GM->mass when available and convert units consistently.
- Emit a concise `create_planet(...)` block or JSON for ingestion by `main.c`.

## Integration & numeric notes (short)
- Use the existing `Vec3` helpers and `DATA`/`SMALL_DATA` types. Keep global positions in higher precision and perform hot physics math in the smaller, faster type for speed and cache locality.
- Allocate and reuse acceleration buffers; avoid per‑frame malloc/free. Use `sqrtf`/`sinf` in SMALL_DATA code paths and mark hot-loop pointers to help compiler optimizations.
- Keep solver logic separated from N‑body verification.


## Short TODO list:
- Change the integrator if need be.
- Optimise the integrator so the runtime is faster
- update so 1x = 1s simulation time / 1 s real time 

## Roadmap (concise)
1. Add optimizer core (candidate generation, scoring, ranking).
2. Implement robust Lambert solver and patched‑conic helpers.
3. Add trajectory simulator to apply impulses and call the N‑body integrator for verification.
4. Expose CLI flags, JSON output and a simple visualization overlay.
5. Add unit tests for numeric correctness and canonical transfer cases.

## Output
- Ranked candidate routes with total Δv, breakdown (departure/mid/arrival), TOF, maneuver times/vectors, and N‑body verification errors.

## License
MIT - Copyright (c) 2026 Dinu Eduard-Alexandru