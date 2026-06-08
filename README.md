# LUNA (LUcas' N-body Analyzer)
3D N-body simulator built in C++20. Implements Barnes-Hut octree and direct summation force solvers with OpenMP and Intel TBB parallelization, symplectic leapfrog integration, and real-time OpenGL rendering. Runs everything on the CPU (for now).

<p align="center">
<img width="640" height="480" alt="luna_cut_compressed" src="https://github.com/user-attachments/assets/f7c46acb-b737-41cc-891e-5fdc0f95a683" />
</p>

## Demos

<img width="480" height="360" alt="demo1_small" src="https://github.com/user-attachments/assets/f3767d47-2c35-4403-99e8-3f530b413ac6" />

[Full quality video demo](https://youtu.be/Y13p_DaeD6w?si=TXdJW2_Yg9dYYGFU)

*10,000 Bodies running at ~160 FPS*

## Running LUNA

Install system dependencies (Fedora/RHEL):
```sh
sudo dnf install gcc-c++ cmake ninja-build mesa-libGL-devel libX11-devel libXrandr-devel libXi-devel libXcursor-devel libXinerama-devel
```

Or on Debian/Ubuntu:
```sh
sudo apt install build-essential cmake libgl-dev libx11-dev libxrandr-dev libxi-dev libxcursor-dev libxinerama-dev
```

Clone the repo:
```sh
git clone --recurse-submodules https://github.com/pants721/luna
```

Run `setup.sh`:
```sh
cd luna
./setup.sh
```
Build:
```sh
./build.sh
```

Run LUNA:
```sh
# GUI (requires a display)
./build/luna_gui examples/luna_text.json
```
```sh
# Headless (no window, no OpenGL required)
./build/luna_headless examples/luna_text.json
```


## Performance
I haven't taken the time to formally measure performance yet, but here are my estimates for FPS at N bodies using TBB Barnes-Hut solver and TBB Leap Frog integrator. I measured these numbers on my PC, which is more performant than average, so the measurements are optimistic.
| N Bodies | FPS |
| --- | --- |
| <7500 | Max |
| 10,000 | 160 |
| 20,000 | 90 |
| 30,000 | 50 |
| 40,000 | 40 |
| 50,000 | 30 |
| 60,000 | 22 |
| 70,000 | 20 |
| 80,000 | 16 |
| 90,000 | 14 |
| 100,000 | 12 |
