# Boid Simulation (OpenGL + C++)

Real-time flocking simulation based on Craig Reynolds' boids model, implemented in C++ with OpenGL rendering and an ImGui control panel.

## Project Description

This project demonstrates emergent behavior from simple local rules:

- Alignment
- Cohesion
- Separation
- Obstacle avoidance

It also includes multiple execution modes to compare performance:

- Serial (single-thread)
- OpenMP (multithreaded CPU update)
- MPI (distributed update with rank 0 rendering)

## Preview

![Boid simulation preview](https://user-images.githubusercontent.com/19228971/170995953-f7a6fd86-b46c-4d9f-89d8-4b71b1fdddbd.gif)

## Tech Stack

- C++17
- OpenGL (GLAD)
- SDL2
- GLM
- Dear ImGui
- CMake

## Repository Structure

- `src/boidSimulation/` application source code
- `include/boidSimulation/` public headers and configuration
- `shaders/` vertex and fragment shaders
- `libs/` vendored third-party libraries (GLAD, GLM, ImGui)

## Build Requirements

- CMake 3.9.1+
- C++17-compatible compiler
- SDL2 development package
- Optional: OpenMP and MPI toolchains

## Build and Run

### Linux or WSL (recommended)

Install dependencies:

```bash
sudo apt update
sudo apt install -y build-essential cmake libsdl2-dev libgl1-mesa-dev libglu1-mesa-dev
```

Build and run:

```bash
bash run.sh
```

Run with MPI (optional):

```bash
RUN_WITH_MPI=1 MPI_PROCS=4 bash run.sh
```

### Windows (MSYS2/MinGW)

Install dependencies in MSYS2 MinGW x64 shell:

```bash
pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-SDL2
```

Configure, build, run:

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
../bin/BoidSimulation.exe
```

## Runtime Controls

Use the ImGui panel to tune simulation and performance settings in real time.
Typical configurable parameters include:

- Boid/species counts
- Rule weights and neighbor distances
- Obstacle count and behavior
- Execution mode selection (Serial/OpenMP/MPI)

## Configuration

Core simulation constants are defined in:

- `include/boidSimulation/Settings/config.h`

## Troubleshooting

- If SDL2 is missing, install the correct development package for your environment.
- If MPI is not found, install OpenMPI (`openmpi-bin`, `libopenmpi-dev`) and rebuild.
- On WSL with source under `/mnt/c/...`, `run.sh` automatically uses a Linux-native build cache directory.

## Reference

- Boids model: https://www.red3d.com/cwr/boids/
