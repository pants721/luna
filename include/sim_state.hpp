#pragma once

#include <cstddef>
#include <utility>
#include <vector>

struct SimState {
    size_t n;

    std::vector<double> mass;

    // position
    std::vector<double> x, y, z;

    // velocity
    std::vector<double> vx, vy, vz;

    // acceleration
    std::vector<double> ax, ay, az;

    SimState(size_t n) : n(n), mass(n), 
        x(n), y(n), z(n), 
        vx(n), vy(n), vz(n),
        ax(n), ay(n), az(n) {}

    SimState(size_t n, std::pair<double, double> mass_range, 
             std::pair<double, double> pos_range);

    // Initialize n bodies with mass and pos within ranges
    SimState(size_t n, std::pair<double, double> mass_range, 
             std::pair<double, double> x_range, 
             std::pair<double, double> y_range,
             std::pair<double, double> z_range);
};

void reset(SimState &state);
void computeForces(SimState &state);
void integrate(SimState &current, SimState &next, double dt);
void step(SimState &current, SimState &next, double dt);
void printState(SimState &state, int step);
