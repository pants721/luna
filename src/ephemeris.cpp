#include "ephemeris.hpp"
#include "constants.hpp"
#include "sim_config.hpp"

#include <algorithm>
#include <execution>
#include <iostream>
#include <numeric>
#include <random>
#include <cmath>
#include <utility>
#include <vector>

Ephemeris::Ephemeris(size_t n, std::pair<double, double> mass_range,
                   std::pair<double, double> pos_range) : Ephemeris(n) {
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

Ephemeris::Ephemeris(size_t n, std::pair<double, double> mass_range, 
         std::pair<double, double> x_range, 
         std::pair<double, double> y_range,
         std::pair<double, double> z_range) : Ephemeris(n) {
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

Ephemeris::Ephemeris(SimConfig config) : Ephemeris(config.num_bodies) {
    n = config.num_bodies;

    // preconfigured bodies
    size_t num_bodies_given = 0;
    if (config.bodies.has_value()) {
        num_bodies_given = config.bodies->size();
        std::vector<BodyInfo> bodies = config.bodies.value();    

        for (size_t i = 0; i < bodies.size(); ++i) {
            BodyInfo body_info = bodies[i];
            mass[i] = body_info.mass;
            x[i] = body_info.initial_pos.x;
            y[i] = body_info.initial_pos.y;
            z[i] = body_info.initial_pos.z;
        }
    }


    // random config
    if (config.random_config.has_value()) {
        RandConfig random_config = config.random_config.value();
        std::pair<double, double> mass_range = random_config.mass_range;
        std::pair<double, double> x_range = random_config.x_range();
        std::pair<double, double> y_range = random_config.y_range();
        std::pair<double, double> z_range = random_config.z_range();

        std::random_device rd;
        std::mt19937 gen(rd());

        std::uniform_real_distribution<> mass_distr(mass_range.first, mass_range.second);
        std::uniform_real_distribution<> x_distr(x_range.first, x_range.second);
        std::uniform_real_distribution<> y_distr(y_range.first, y_range.second);
        std::uniform_real_distribution<> z_distr(z_range.first, z_range.second);

        // randomly generate n bodies that havent been given yet
        for (size_t i = num_bodies_given; i < n; ++i) {
            mass[i] = mass_distr(gen);
            x[i] = x_distr(gen);
            y[i] = y_distr(gen);
            z[i] = z_distr(gen);
        }
    } else if (num_bodies_given != n) { // no random config and not enough bodies given so shrink n
        n = num_bodies_given;
    }
}

void computeForces(Ephemeris &s) {
    // Create a vector of indices to parallelize over
    std::vector<size_t> indices(s.n);
    std::iota(indices.begin(), indices.end(), 0);

    // Parallel + Vectorized execution policy
    std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](size_t i) {
        float ax = 0, ay = 0, az = 0;
        float xi = (float)s.x[i], yi = (float)s.y[i], zi = (float)s.z[i];
        float mi = (float)s.mass[i];

        #pragma omp simd
        for (size_t j = 0; j < s.n; ++j) {
            if (i == j) continue;
            float xj = (float)s.x[j], yj = (float)s.y[j], zj = (float)s.z[j];
            float mj = (float)s.mass[j];

            float dx = xj - xi;
            float dy = yj - yi;
            float dz = zj - zi;

            float dist_sq = (dx * dx + dy * dy + dz * dz) + SOFTENING;

            // hardware approximation of 1/sqrt(dist_sq)
            __m128 reg_dist_sq = _mm_set_ss(dist_sq);
            __m128 reg_rsqrt = _mm_rsqrt_ss(reg_dist_sq);
            float r_inv = _mm_cvtss_f32(reg_rsqrt);

            // Newton-Raphson Refinement
            // doubles the precision of the approximation
            r_inv = r_inv * (1.5f - 0.5f * dist_sq * r_inv * r_inv);

            float r_inv_sq = r_inv * r_inv;
            float r_inv_cub = r_inv_sq * r_inv;

            float common = (float)G * mj * r_inv_cub;
            ax += dx * common;
            ay += dy * common;
            az += dz * common;
        }

        s.ax[i] = (double)ax;
        s.ay[i] = (double)ay;
        s.az[i] = (double)az;
    });
}

void integrate(Ephemeris &current, Ephemeris &next, double dt) {
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

        if (std::isnan(next.x[i])) std::cout << "EXPLOSION!" << '\n';
    });
}

void finalKick(Ephemeris &current, Ephemeris &next, double dt) {
    std::vector<size_t> indices(current.n);
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](size_t i) {
        next.vx[i] += 0.5 * next.ax[i] * dt;
        next.vy[i] += 0.5 * next.ay[i] * dt;
        next.vz[i] += 0.5 * next.az[i] * dt;
    });
}

void step(Ephemeris &current, Ephemeris &next, double dt) {
    computeForces(current);
    integrate(current, next, dt);
    computeForces(next);
    finalKick(current, next, dt);
    std::swap(current, next);
}

void printState(Ephemeris &state, int step) {
    for (size_t i = 0; i < state.n; ++i) {
        printf("%d,%zu,%f,%f,%f\n", step, i, state.x[i], state.y[i], state.z[i]);
    }
}
