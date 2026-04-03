#include "sim_state.hpp"
#include "constants.hpp"

#include <algorithm>
#include <execution>
#include <numeric>
#include <random>
#include <cmath>
#include <vector>

SimState::SimState(size_t n, std::pair<double, double> mass_range,
                   std::pair<double, double> pos_range) : SimState(n) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<> mass_distr(mass_range.first, mass_range.second);
    std::uniform_real_distribution<> pos_distr(pos_range.first, pos_range.second);

    for (size_t i = 0; i < n; ++i) {
        mass[i] = mass_distr(gen);
        x[i] = pos_distr(gen);
        y[i] = pos_distr(gen);
        z[i] = pos_distr(gen);
    }
}

SimState::SimState(size_t n, std::pair<double, double> mass_range, 
         std::pair<double, double> x_range, 
         std::pair<double, double> y_range,
         std::pair<double, double> z_range) : SimState(n) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<> mass_distr(mass_range.first, mass_range.second);
    std::uniform_real_distribution<> x_distr(x_range.first, x_range.second);
    std::uniform_real_distribution<> y_distr(y_range.first, y_range.second);
    std::uniform_real_distribution<> z_distr(z_range.first, z_range.second);

    for (size_t i = 0; i < n; ++i) {
        mass[i] = mass_distr(gen);
        x[i] = x_distr(gen);
        y[i] = y_distr(gen);
        z[i] = z_distr(gen);
    }
}

void computeForces(SimState &s) {
    // Create a vector of indices to parallelize over
    std::vector<size_t> indices(s.n);
    std::iota(indices.begin(), indices.end(), 0);

    // Parallel + Vectorized execution policy
    std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](size_t i) {
        double ax = 0, ay = 0, az = 0;
        double xi = s.x[i], yi = s.y[i], zi = s.z[i];

        #pragma omp simd
        for (size_t j = 0; j < s.n; ++j) {
            if (i == j) continue;

            double dx = s.x[j] - xi;
            double dy = s.y[j] - yi;
            double dz = s.z[j] - zi;

            double r_sq = (dx * dx + dy * dy + dz * dz) + SOFTENING;
            double r_inv_cub = 1.0 / (std::sqrt(r_sq) * r_sq);

            double common = G * s.mass[j] * r_inv_cub;
            ax += dx * common;
            ay += dy * common;
            az += dz * common;
        }

        s.ax[i] = ax;
        s.ay[i] = ay;
        s.az[i] = az;
    });
}

void integrate(SimState &current, SimState &next, double dt) {
    std::vector<size_t> indices(current.n);
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](size_t i) {
        // Kick (Update velocity by half-step)
        double vx_half = current.vx[i] + 0.5 * current.ax[i] * dt;
        double vy_half = current.vy[i] + 0.5 * current.ay[i] * dt;
        double vz_half = current.vz[i] + 0.5 * current.az[i] * dt;

        // Drift (Update position using half-step velocity)
        next.x[i] = current.x[i] + vx_half * dt;
        next.y[i] = current.y[i] + vy_half * dt;
        next.z[i] = current.z[i] + vz_half * dt;

        // Copy static data
        next.mass[i] = current.mass[i];
        
        // Final Kick happens in next step once new forces are computed
        next.vx[i] = vx_half; 
        next.vy[i] = vy_half;
        next.vz[i] = vz_half;
    });
}

void step(SimState &current, SimState &next, double dt) {
    computeForces(current);
    integrate(current, next, dt);
    // After integrate, the velocities in 'next' are only half-kicked.
    // A full Velocity-Verlet would compute forces again here for the second half-kick.
    std::swap(current, next);
}

void printState(SimState &state, int step) {
    for (size_t i = 0; i < state.n; ++i) {
        printf("%d,%zu,%f,%f,%f\n", step, i, state.x[i], state.y[i], state.z[i]);
    }
}
