#pragma once

#include <cstddef>
#include <utility>
#include <vector>

struct Ephemeris {
    size_t n;

    std::vector<double> mass;

    // position
    std::vector<double> x, y, z;

    // velocity
    std::vector<double> vx, vy, vz;

    // acceleration
    std::vector<double> ax, ay, az;

    Ephemeris(size_t n) : n(n), mass(n), 
        x(n), y(n), z(n), 
        vx(n), vy(n), vz(n),
        ax(n), ay(n), az(n) {}

    Ephemeris(size_t n, std::pair<double, double> mass_range, 
             std::pair<double, double> pos_range);

    // Initialize n bodies with mass and pos within ranges
    Ephemeris(size_t n, std::pair<double, double> mass_range, 
             std::pair<double, double> x_range, 
             std::pair<double, double> y_range,
             std::pair<double, double> z_range);
};

void reset(Ephemeris &state);
void computeForces(Ephemeris &state);
void integrate(Ephemeris &current, Ephemeris &next, double dt);
void step(Ephemeris &current, Ephemeris &next, double dt);
void printState(Ephemeris &state, int step);
