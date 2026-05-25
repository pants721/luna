#include "integrators/leapfrog_openmp.hpp"

#include "physics/ephemeris.hpp"

void integrators::LeapFrogOpenMP::preForceUpdate(physics::Ephemeris &current, 
                                                 physics::Ephemeris &next,
                                                 double dt) {
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

void integrators::LeapFrogOpenMP::postForceUpdate(physics::Ephemeris &current, 
                                                 physics::Ephemeris &next,
                                                 double dt) {
    #pragma omp parallel for schedule(guided)
    for (size_t i = 0; i < current.n; ++i) {
        next.vx[i] += 0.5 * next.ax[i] * dt;
        next.vy[i] += 0.5 * next.ay[i] * dt;
        next.vz[i] += 0.5 * next.az[i] * dt;
    }
}
