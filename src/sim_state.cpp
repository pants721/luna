#include "sim_state.hpp"

#include "constants.hpp"

#include <random>

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

void reset(SimState &state) {
    // reset acceleration vectors
    std::fill(state.ax.begin(), state.ax.end(), 0.0);
    std::fill(state.ay.begin(), state.ay.end(), 0.0);
    std::fill(state.az.begin(), state.az.end(), 0.0);
}

void compute(SimState &state) {
    for (size_t i = 0; i < state.n; ++i) {
        for (size_t j = 0; j < state.n; ++j) {
            double mi = state.mass[i];
            double mj = state.mass[j];

            double xi = state.x[i];
            double yi = state.y[i];
            double zi = state.z[i];

            double xj = state.x[j];
            double yj = state.y[j];
            double zj = state.z[j];

            // calculate r_ij
            double dx = xj - xi;
            double dy = yj - yi;
            double dz = zj - zi;

            double r_sq = (dx * dx + dy * dy + dz * dz) + SOFTENING;
            double r_cub = sqrt(r_sq) * r_sq;

            // Newton's law of universal gravitation: https://en.wikipedia.org/wiki/Newton%27s_law_of_universal_gravitation
            // calculate a_i
            state.ax[i] += G * mj * dx / r_cub;
            state.ay[i] += G * mj * dy / r_cub;
            state.az[i] += G * mj * dz / r_cub;

        }
    }
}

void integrate(SimState &state, double dt) {
    for (size_t i = 0; i < state.n; ++i) {
        // kick
        state.vx[i] += 0.5 * state.ax[i] * dt;
        state.vy[i] += 0.5 * state.ay[i] * dt;
        state.vz[i] += 0.5 * state.az[i] * dt;

        // drift
        state.x[i] += state.vx[i] * dt;
        state.y[i] += state.vy[i] * dt;
        state.z[i] += state.vz[i] * dt;

        // kick
        state.vx[i] += 0.5 * state.ax[i] * dt;
        state.vy[i] += 0.5 * state.ay[i] * dt;
        state.vz[i] += 0.5 * state.az[i] * dt;

    }

}

void step(SimState &state, double dt) {
    reset(state);
    compute(state);
    integrate(state, dt);
}

// prints state at each step in format "step, i, x, y, z"
void printState(SimState &state, int step) {
    for (size_t i = 0; i < state.n; ++i) {
        printf("%d,%zu,%f,%f,%f\n", step, i, state.x[i], state.y[i], state.z[i]);
    }
}
