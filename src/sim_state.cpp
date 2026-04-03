#include "sim_state.hpp"

#include "constants.hpp"

#include <cstddef>
#include <iterator>
#include <random>
#include <thread>
#include <utility>
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
    size_t n_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    size_t chunk_size = s.n / n_threads;

    auto compute_forces_range = [&](size_t start, size_t end) {
        for (size_t i = start; i < end; ++i) {
            double ax = 0, ay = 0, az = 0;
            #pragma omp simd
            for (size_t j = 0; j < s.n; ++j) {
                double mi = s.mass[i];
                double mj = s.mass[j];

                double xi = s.x[i];
                double yi = s.y[i];
                double zi = s.z[i];

                double xj = s.x[j];
                double yj = s.y[j];
                double zj = s.z[j];

                // calculate r_ij
                double dx = xj - xi;
                double dy = yj - yi;
                double dz = zj - zi;

                double r_sq = (dx * dx + dy * dy + dz * dz) + SOFTENING;
                double r_inv_cub = 1.0 / (sqrt(r_sq) * r_sq);

                // Newton's law of universal gravitation: https://en.wikipedia.org/wiki/Newton%27s_law_of_universal_gravitation
                // calculate a_i
                ax += G * mj * dx * r_inv_cub;
                ay += G * mj * dy * r_inv_cub;
                az += G * mj * dz * r_inv_cub;
            }

            s.ax[i] = ax;
            s.ay[i] = ay;
            s.az[i] = az;
        }
    };

    for (size_t t = 0; t < n_threads; t++) {
        size_t start = t * chunk_size;
        size_t end = std::min(start + chunk_size, s.n);
        if (start < end) {
            threads.emplace_back(compute_forces_range, start, end);
        }
    }

    for (auto &th : threads) {
        th.join();
    }
}

void integrate(SimState &current, SimState &next, double dt) {
    size_t n = current.n;
    size_t n_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    size_t chunk_size = n / n_threads;

    auto integrate_range = [&](size_t start, size_t end) {
        for (size_t i = start; i < end; ++i) {
            // kick
            double vx = current.vx[i] + 0.5 * current.ax[i] * dt;
            double vy = current.vy[i] + 0.5 * current.ay[i] * dt;
            double vz = current.vz[i] + 0.5 * current.az[i] * dt;

            // drift
            double x = current.x[i] + current.vx[i] * dt;
            double y = current.y[i] + current.vy[i] * dt;
            double z = current.z[i] + current.vz[i] * dt;

            // kick
            vx = current.vx[i] + 0.5 * current.ax[i] * dt;
            vy = current.vy[i] + 0.5 * current.ay[i] * dt;
            vz = current.vz[i] + 0.5 * current.az[i] * dt;

            // write to next
            next.x[i] = x;
            next.y[i] = y;
            next.z[i] = z;

            next.vx[i] = vx;
            next.vy[i] = vy;
            next.vz[i] = vz;

            next.ax[i] = current.ax[i];
            next.ay[i] = current.ay[i];
            next.az[i] = current.az[i];

            next.mass[i] = current.mass[i];
        }
    };

    for (size_t t = 0; t < n_threads; t++) {
        size_t start = t * chunk_size;
        size_t end = std::min(start + chunk_size, n);
        if (start < end) {
            threads.emplace_back(integrate_range, start, end);
        }
    }

    for (auto &th : threads) {
        th.join();
    }
}

void step(SimState &current, SimState &next, double dt) {
    computeForces(current);
    integrate(current, next, dt);
    std::swap(current, next);
}

// prints state at each step in format "step, i, x, y, z"
void printState(SimState &state, int step) {
    for (size_t i = 0; i < state.n; ++i) {
        printf("%d,%zu,%f,%f,%f\n", step, i, state.x[i], state.y[i], state.z[i]);
    }
}
