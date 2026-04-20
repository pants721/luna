#include "physics/ephemeris.hpp"
#include "constants.hpp"
#include "cfg/sim_config.hpp"
#include "octree.hpp"

#include <algorithm>
#include <execution>
#include <iostream>
#include <iterator>
#include <limits>
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

void physics::computeBounds(Ephemeris &s) {
    double max_x = -std::numeric_limits<double>::infinity();
    double max_y = -std::numeric_limits<double>::infinity();
    double max_z = -std::numeric_limits<double>::infinity();

    double min_x = std::numeric_limits<double>::infinity();
    double min_y = std::numeric_limits<double>::infinity();
    double min_z = std::numeric_limits<double>::infinity();

#ifdef ENABLE_OMP
    #pragma omp parallel for \
        reduction(max:max_x,max_y,max_z) \
        reduction(min:min_x,min_y,min_z)
#endif
    for (size_t i = 0; i < s.n; ++i) {
        double x = s.x[i];
        double y = s.y[i];
        double z = s.z[i];

        max_x = std::max(max_x, x); // max x
        max_y = std::max(max_y, y); // max y
        max_z = std::max(max_z, z); // max z

        min_x = std::min(min_x, x); // min x
        min_y = std::min(min_y, y); // min y
        min_z = std::min(min_z, z); // min z
    }

    s.max_x = max_x;
    s.max_y = max_y;
    s.max_z = max_z;

    s.min_x = min_x;
    s.min_y = min_y;
    s.min_z = min_z;
}

void physics::computeAccelDirect(Ephemeris &s) {
#ifdef ENABLE_OMP
    #pragma omp parallel for schedule(static)
#endif
    for (size_t i = 0; i < s.n; ++i) {
        double ax = 0.0;
        double ay = 0.0;
        double az = 0.0;

        double xi = s.x[i];
        double yi = s.y[i];
        double zi = s.z[i];

#ifdef ENABLE_OMP
        // allow parallel work to compute a delta for acceleration independently 
        // and combine
        #pragma omp simd reduction(+:ax,ay,az)
#endif
        for (size_t j = 0; j < s.n; ++j) {
            double xj = s.x[j], yj = s.y[j], zj = s.z[j];
            double mj = s.mass[j];

            double dx = xj - xi;
            double dy = yj - yi;
            double dz = zj - zi;

            double dist_sq = (dx * dx + dy * dy + dz * dz) + SOFTENING;

            double r_inv = 1.0 / std::sqrt(dist_sq);

            double r_inv_sq = r_inv * r_inv;
            double r_inv_cub = r_inv_sq * r_inv;

            // no early return bcs vectorization
            if (i != j) {
                double common = G * mj * r_inv_cub;
                ax += dx * common;
                ay += dy * common;
                az += dz * common;
            }
        }

        s.ax[i] = ax;
        s.ay[i] = ay;
        s.az[i] = az;
    }
}

void physics::computeAccelBH(Ephemeris &s) {
    physics::Octree tree(&s);
    tree.build();
    tree.computeMass();

    // reset accels
    std::fill(s.ax.begin(), s.ax.end(), 0.0);
    std::fill(s.ay.begin(), s.ay.end(), 0.0);
    std::fill(s.az.begin(), s.az.end(), 0.0);

#ifdef ENABLE_OMP
    #pragma omp parallel for schedule(dynamic, 64)
#endif
    for (size_t i = 0; i < s.n; ++i) {
        tree.computeAccelIt(i, BH_THETA);
    }
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

void physics::integrate(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    for (size_t i = 0; i < current.n; ++i) {
        integrateSingle(current, next, dt, i);
    }
}

void integrateMT(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
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

void physics::finalKick(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    for (size_t i = 0; i < current.n; ++i) {
        finalKickSingle(current, next, dt, i);
    }
}
void finalKickMT(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    std::vector<size_t> indices(current.n);
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](size_t i) {
        finalKickSingle(current, next, dt, i);
    });
}

void physics::stepDirect(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    computeAccelDirect(current);
    integrate(current, next, dt);
    computeAccelDirect(next);
    finalKick(current, next, dt);
    std::swap(current, next);
}

void stepDirectMT(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    computeAccelDirect(current);
    integrateMT(current, next, dt);
    computeAccelDirect(next);
    finalKickMT(current, next, dt);
    std::swap(current, next);
}

void physics::stepBH(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    computeBounds(current);
    computeAccelBH(current);
    integrate(current, next, dt);
    computeAccelBH(next);
    finalKick(current, next, dt);
    std::swap(current, next);
}

void stepBHMT(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    computeBounds(current);
    computeAccelBH(current);
    integrateMT(current, next, dt);
    computeAccelBH(next);
    finalKickMT(current, next, dt);
    std::swap(current, next);
}

void physics::printState(physics::Ephemeris &state, int step) {
    for (size_t i = 0; i < state.n; ++i) {
        printf("%d,%zu,%f,%f,%f\n", step, i, state.x[i], state.y[i], state.z[i]);
    }
}
