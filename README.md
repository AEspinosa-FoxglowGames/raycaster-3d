# raycaster-3d

A pseudo-3D dungeon crawler built from scratch in C++ and OpenGL — no engine.

Rendering is based on the DDA (Digital Differential Analysis) raycasting algorithm,
the same technique used in Wolfenstein 3D. Every rendering system, texture sampler,
and game loop is written manually without relying on a game framework.

## Status

🚧 Early development — renderer working, gameplay systems in progress.

## Technical details

- **Language:** C++
- **Graphics:** OpenGL (no engine, no framework)
- **Renderer:** Column-based DDA raycasting with texture mapping and decal support
- **Build system:** CMake

## What's implemented

- DDA ray-plane intersection and column rendering
- Texture mapping on walls
- Decal rendering system
- Basic entity and HUD structure

## Building

Requires OpenGL and CMake. Clone the repo and open `CMakeLists.txt` in Visual Studio
or build via CMake from the command line.

## About

Built by [Austin Espinosa](https://foxglowgames.com) — indie developer and
Information Science student at ICU Tokyo.
