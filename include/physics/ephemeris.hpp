#pragma once

#include "cfg/sim_config.hpp"

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

namespace physics {

struct Ephemeris {
    size_t n;

    std::vector<double> mass;

    // position
    std::vector<double> x, y, z;

    // velocity
    std::vector<double> vx, vy, vz;

    // acceleration
    std::vector<double> ax, ay, az;

    double min_x = INFINITY; 
    double min_y = INFINITY; 
    double min_z = INFINITY;

    double max_x = -INFINITY;
    double max_y = -INFINITY;
    double max_z = -INFINITY;

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

    Ephemeris(cfg::SimConfig config);
};

void reset(Ephemeris &state);

void resetBounds(Ephemeris &s);
void computeBounds(Ephemeris &s);

void computeForcesDirectSingle(Ephemeris &s, size_t b_idx);
void computeForcesDirect(Ephemeris &state);

void computeForcesBH(Ephemeris &state);

void integrateSingle(Ephemeris &current, Ephemeris &next, double dt, size_t b_idx);
void integrate(Ephemeris &current, Ephemeris &next, double dt);

void finalKickSingle(Ephemeris &current, Ephemeris &next, double dt, size_t b_idx);
void finalKick(Ephemeris &current, Ephemeris &next, double dt);

void stepDirect(Ephemeris &current, Ephemeris &next, double dt);
void stepBH(Ephemeris &current, Ephemeris &next, double dt);
void printState(Ephemeris &state, int step);

}
