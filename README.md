# raycaster-3d
A pseudo-3D dungeon crawler built from scratch in C++ and OpenGL — no engine.
Rendering is based on the DDA (Digital Differential Analysis) raycasting algorithm,
the same technique used in Wolfenstein 3D. Every rendering system, texture sampler,
and game loop is written manually without relying on a game framework.

## Status
🚧 Early development — renderer and core systems working, gameplay in progress.

## Technical details
- **Language:** C++20
- **Graphics:** OpenGL (no engine, no framework)
- **Renderer:** Column-based DDA raycasting with per-column texture mapping
- **Textures:** CPU-side sampling via stb_image — no GPU texture units used for world rendering
- **Build system:** CMake + vcpkg

## What's implemented
- DDA raycasting with N/S vs E/W face shading for depth perception
- Distance-based fog/shading on walls, floor, and ceiling
- Textured walls with multiple tile types (stone, brick, concrete, sealed)
- Textured floor and ceiling with perspective-correct ray projection
- Door system — horizontal and vertical orientations, recessed half-width rendering with frame strips, locked variant
- Wall-face decal system — entities rendered as texture overlays on specific wall faces
- Tile registry with typed enum (`TileType`) covering walls, doors, open passages, and zone triggers
- Entity system with type, stage, and interaction flags (notes, monster scaffold)
- Player movement with sprint, stamina drain/regen, and exhaustion threshold
- Slide-along-wall collision (separate X/Y axis testing)
- HUD scaffold — crosshair and health/stamina bars
- Mouse look with cursor lock
- World validation — flags entities placed inside solid tiles at startup

## Controls
| Key | Action |
|---|---|
| `W/A/S/D` | Move |
| `Mouse` | Look |
| `Shift` | Sprint |
| `Escape` | Quit |

## Building
Requires OpenGL and CMake. Clone the repo and open `CMakeLists.txt` in Visual Studio
or build via CMake from the command line. Dependencies managed via vcpkg (`vcpkg.json` included).

## About
Built by [Austin Espinosa](https://foxglowgames.com) — indie developer and
Information Science student at ICU Tokyo.
