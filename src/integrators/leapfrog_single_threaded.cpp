#include "integrators/leapfrog_single_threaded.hpp"

#include "physics/ephemeris.hpp"

void integrators::LeapFrogSingleThreaded::preForceUpdate(physics::Ephemeris &current,
                                                         physics::Ephemeris &next,
                                                         double dt) {
    for (size_t i = 0; i < current.n; ++i) {
        next.vx[i] = current.vx[i] + 0.5 * current.ax[i] * dt;
        next.vy[i] = current.vy[i] + 0.5 * current.ay[i] * dt;
        next.vz[i] = current.vz[i] + 0.5 * current.az[i] * dt;

        next.x[i] = current.x[i] + next.vx[i] * dt;
        next.y[i] = current.y[i] + next.vy[i] * dt;
        next.z[i] = current.z[i] + next.vz[i] * dt;

        next.mass[i] = current.mass[i];
    }
}

void integrators::LeapFrogSingleThreaded::postForceUpdate(physics::Ephemeris &current,
                                                          physics::Ephemeris &next,
                                                          double dt) {
    for (size_t i = 0; i < current.n; ++i) {
        next.vx[i] += 0.5 * next.ax[i] * dt;
        next.vy[i] += 0.5 * next.ay[i] * dt;
        next.vz[i] += 0.5 * next.az[i] * dt;
    }
}
