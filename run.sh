#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Building under /mnt/c in WSL can trigger permission issues with CMake temp files.
# Use a Linux-native writable build dir in that case.
if [[ "$SCRIPT_DIR" == /mnt/* ]]; then
	BUILD_DIR="$HOME/.cache/boid-simulation-opengl/build"
else
	BUILD_DIR="$SCRIPT_DIR/build"
fi

mkdir -p "$BUILD_DIR"
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" -j

if [[ "${RUN_WITH_MPI:-0}" == "1" ]]; then
	MPI_PROCS="${MPI_PROCS:-4}"
	mpirun -np "$MPI_PROCS" "$BUILD_DIR/../bin/BoidSimulation"
else
	"$BUILD_DIR/../bin/BoidSimulation"
fi
