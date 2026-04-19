#include "physics/ephemeris.hpp"
#include "constants.hpp"
#include "cfg/sim_config.hpp"
#include "octree.hpp"

#include <algorithm>
#include <execution>
#include <iostream>
#include <numeric>
#include <random>
#include <cmath>
#include <utility>
#include <vector>

physics::Ephemeris::Ephemeris(size_t n, std::pair<double, double> mass_range,
                   std::pair<double, double> pos_range) : physics::Ephemeris(n) {
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

physics::Ephemeris::Ephemeris(size_t n, std::pair<double, double> mass_range, 
         std::pair<double, double> x_range, 
         std::pair<double, double> y_range,
         std::pair<double, double> z_range) : physics::Ephemeris(n) {
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

physics::Ephemeris::Ephemeris(cfg::SimConfig config) : physics::Ephemeris(config.num_bodies) {
    using namespace cfg;

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

            if (body_info.initial_vel.has_value()) {
                auto initial_vel = body_info.initial_vel.value();
                vx[i] = initial_vel.x;
                vy[i] = initial_vel.y;
                vz[i] = initial_vel.z;
            }
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

void physics::resetBounds(physics::Ephemeris &s) {
    s.max_x = -INFINITY;
    s.max_y = -INFINITY;
    s.max_z = -INFINITY;

    s.min_x = INFINITY;
    s.min_y = INFINITY;
    s.min_z = INFINITY;
}

void physics::computeBoundsSingle(Ephemeris &s, size_t b_idx) {
    double x = s.x[b_idx];
    double y = s.y[b_idx];
    double z = s.z[b_idx];

    // update maxes
    if (x > s.max_x) s.max_x = x;
    if (y > s.max_y) s.max_y = y;
    if (z > s.max_z) s.max_z = z;

    // update mins
    if (x < s.min_x) s.min_x = x;
    if (y < s.min_y) s.min_y = y;
    if (z < s.min_z) s.min_z = z;
}

void physics::computeBoundsST(physics::Ephemeris &s) {
    resetBounds(s);
    for (size_t i = 0; i < s.n; ++i) {
        computeBoundsSingle(s, i);
    }
}

void physics::computeBoundsMT(Ephemeris &s) {
    resetBounds(s);
    std::vector<size_t> indices(s.n);
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), 
                  [&](size_t i) {
        computeBoundsSingle(s, i);
    });
}

void physics::computeForcesDirectSingle(physics::Ephemeris &s, size_t b_idx) {
    float ax = 0, ay = 0, az = 0;
    float xi = (float)s.x[b_idx], yi = (float)s.y[b_idx], zi = (float)s.z[b_idx];
    float mi = (float)s.mass[b_idx];

    #pragma omp simd
    for (size_t j = 0; j < s.n; ++j) {
        if (b_idx == j) continue;
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

    s.ax[b_idx] = (double)ax;
    s.ay[b_idx] = (double)ay;
    s.az[b_idx] = (double)az;
}

void physics::computeForcesDirectST(Ephemeris &s) {
    for (size_t i = 0; i < s.n; ++i) {
        computeForcesDirectSingle(s, i);
    }
}

void physics::computeForcesDirectMT(physics::Ephemeris &s) {
    // // Create a vector of indices to parallelize over
    std::vector<size_t> indices(s.n);
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](size_t i) {
        computeForcesDirectSingle(s, i);
    });
}

void physics::computeForcesBHST(Ephemeris &s) {
    physics::Octree tree(&s);
    tree.build();
    tree.computeMass();

    #pragma omp parallel for schedule(dynamic, 64)
    for (size_t i = 0; i < s.n; ++i) {
        s.ax[i] = 0;
        s.ay[i] = 0;
        s.az[i] = 0;

        tree.computeNetForce(i, BH_THETA);
    }
}

void physics::computeForcesBHMT(physics::Ephemeris &s) {
    physics::Octree tree(&s);
    tree.build();
    tree.computeMass();

    std::vector<size_t> indices(s.n);
    std::iota(indices.begin(), indices.end(), 0);

    // compute accel
    std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](size_t i) {
        s.ax[i] = 0;
        s.ay[i] = 0;
        s.az[i] = 0;

        tree.computeNetForce(i, BH_THETA);
    });
}

void physics::integrateSingle(Ephemeris &current, Ephemeris &next, double dt, size_t b_idx) {
    // Kick (Update velocity by half-step)
    double vx_half = current.vx[b_idx] + 0.5 * current.ax[b_idx] * dt;
    double vy_half = current.vy[b_idx] + 0.5 * current.ay[b_idx] * dt;
    double vz_half = current.vz[b_idx] + 0.5 * current.az[b_idx] * dt;

    // Drift (Update position using half-step velocity)
    next.x[b_idx] = current.x[b_idx] + vx_half * dt;
    next.y[b_idx] = current.y[b_idx] + vy_half * dt;
    next.z[b_idx] = current.z[b_idx] + vz_half * dt;

    // Copy static data
    next.mass[b_idx] = current.mass[b_idx];

    // Final Kick happens in next step once new forces are computed
    next.vx[b_idx] = vx_half; 
    next.vy[b_idx] = vy_half;
    next.vz[b_idx] = vz_half;

    if (std::isnan(next.x[b_idx])) std::cout << "EXPLOSION!" << '\n';
}

void physics::integrateST(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    for (size_t i = 0; i < current.n; ++i) {
        integrateSingle(current, next, dt, i);
    }
}

void physics::integrateMT(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    std::vector<size_t> indices(current.n);
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](size_t i) {
        integrateSingle(current, next, dt, i);
    });
}

void physics::finalKickSingle(physics::Ephemeris &current, physics::Ephemeris &next, double dt, size_t b_idx) {
    next.vx[b_idx] += 0.5 * next.ax[b_idx] * dt;
    next.vy[b_idx] += 0.5 * next.ay[b_idx] * dt;
    next.vz[b_idx] += 0.5 * next.az[b_idx] * dt;
}

void physics::finalKickST(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    for (size_t i = 0; i < current.n; ++i) {
        finalKickSingle(current, next, dt, i);
    }
}
void physics::finalKickMT(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    std::vector<size_t> indices(current.n);
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](size_t i) {
        finalKickSingle(current, next, dt, i);
    });
}

void physics::stepDirectST(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    computeForcesDirectST(current);
    integrateST(current, next, dt);
    computeForcesDirectST(next);
    finalKickST(current, next, dt);
    std::swap(current, next);
}

void physics::stepDirectMT(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    computeForcesDirectMT(current);
    integrateMT(current, next, dt);
    computeForcesDirectMT(next);
    finalKickMT(current, next, dt);
    std::swap(current, next);
}

void physics::stepBHST(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    computeBoundsST(current);
    computeForcesBHST(current);
    integrateST(current, next, dt);
    computeForcesBHST(next);
    finalKickST(current, next, dt);
    std::swap(current, next);
}

void physics::stepBHMT(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    computeBoundsMT(current);
    computeForcesBHMT(current);
    integrateMT(current, next, dt);
    computeForcesBHMT(next);
    finalKickMT(current, next, dt);
    std::swap(current, next);
}

void physics::printState(physics::Ephemeris &state, int step) {
    for (size_t i = 0; i < state.n; ++i) {
        printf("%d,%zu,%f,%f,%f\n", step, i, state.x[i], state.y[i], state.z[i]);
    }
}
