#!/bin/sh
# gcc -o build/rl_grid_sim.out src/main.c -lraylib -lm -lX11 # replace with calling make buildfile
mkdir -p build
make
./build/rl_grid_sim
