#include "sim/step.hpp"

#include "physics/ephemeris.hpp"
#include "constants.hpp"
#include "cfg/sim_config.hpp"
#include "octree.hpp"

#include <algorithm>
#include <cstdio>
#include <execution>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <random>
#include <cmath>
#include <utility>
#include <vector>

#include <omp.h>

void sim::resetAccel(physics::Ephemeris &s) {
    // reset accels
    std::fill(s.ax.begin(), s.ax.end(), 0.0);
    std::fill(s.ay.begin(), s.ay.end(), 0.0);
    std::fill(s.az.begin(), s.az.end(), 0.0);
}

void sim::computeBounds(physics::Ephemeris &s) {
    double max_x = -std::numeric_limits<double>::infinity();
    double max_y = -std::numeric_limits<double>::infinity();
    double max_z = -std::numeric_limits<double>::infinity();

    double min_x = std::numeric_limits<double>::infinity();
    double min_y = std::numeric_limits<double>::infinity();
    double min_z = std::numeric_limits<double>::infinity();

    #pragma omp parallel for \
        reduction(max:max_x,max_y,max_z) \
        reduction(min:min_x,min_y,min_z)
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

void sim::computeAccelDirect(physics::Ephemeris &s) {
    #pragma omp parallel for schedule(guided)
    for (size_t i = 0; i < s.n; ++i) {
        double ax = 0.0;
        double ay = 0.0;
        double az = 0.0;

        double xi = s.x[i];
        double yi = s.y[i];
        double zi = s.z[i];

        // allow parallel work to compute a delta for acceleration independently 
        // and combine
        #pragma omp simd reduction(+:ax,ay,az)
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

void sim::computeAccelBH(physics::Ephemeris &s) {
    physics::Octree tree(&s);
    tree.build();
    tree.computeMass();

    resetAccel(s);

    #pragma omp parallel for schedule(guided)
    for (size_t i = 0; i < s.n; ++i) {
        tree.computeAccelIt(i, BH_THETA);
    }
}

void sim::integrate(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    #pragma omp parallel for schedule(guided)
    for (size_t i = 0; i < current.n; ++i) {
        // kick
        // v(t + dt/2) (half-step velocity)
        next.vx[i] = current.vx[i] + 0.5 * current.ax[i] * dt;
        next.vy[i] = current.vy[i] + 0.5 * current.ay[i] * dt;
        next.vz[i] = current.vz[i] + 0.5 * current.az[i] * dt;

        // drift (update position using half-step velocity)
        next.x[i] = current.x[i] + next.vx[i];
        next.y[i] = current.y[i] + next.vy[i];
        next.z[i] = current.z[i] + next.vz[i];

        next.mass[i] = current.mass[i];
    }
}

void sim::halfKick(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    #pragma omp parallel for schedule(guided)
    for (size_t i = 0; i < current.n; ++i) {
        next.vx[i] += 0.5 * next.ax[i] * dt;
        next.vy[i] += 0.5 * next.ay[i] * dt;
        next.vz[i] += 0.5 * next.az[i] * dt;
    }
}


void sim::force_policy::Direct::step(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    using namespace sim;
    computeAccelDirect(current);
    integrate(current, next, dt);
    computeAccelDirect(next);
    halfKick(current, next, dt);
    std::swap(current, next);
}

void sim::force_policy::BarnesHut::step(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    using namespace sim;
    computeBounds(current);
    computeAccelBH(current);
    integrate(current, next, dt);
    computeAccelBH(next);
    halfKick(current, next, dt);
    std::swap(current, next);
}
