#include "integrators/leapfrog_tbb.hpp"
#include "physics/ephemeris.hpp"

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

void integrators::LeapFrogTBB::preForceUpdate(physics::Ephemeris &current, 
                                              physics::Ephemeris &next,
                                              double dt) {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, current.n), 
    [&](const tbb::blocked_range<size_t>& range) {
        
        for (size_t i = range.begin(); i != range.end(); ++i) {
            next.vx[i] = current.vx[i] + 0.5 * current.ax[i] * dt;
            next.vy[i] = current.vy[i] + 0.5 * current.ay[i] * dt;
            next.vz[i] = current.vz[i] + 0.5 * current.az[i] * dt;

            next.x[i] = current.x[i] + next.vx[i];
            next.y[i] = current.y[i] + next.vy[i];
            next.z[i] = current.z[i] + next.vz[i];

            next.mass[i] = current.mass[i];
        }
    });
}

void integrators::LeapFrogTBB::postForceUpdate(physics::Ephemeris &current, 
                                               physics::Ephemeris &next,
                                               double dt) {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, current.n), 
    [&](const tbb::blocked_range<size_t>& range) {
        
        for (size_t i = range.begin(); i != range.end(); ++i) {
            next.vx[i] += 0.5 * next.ax[i] * dt;
            next.vy[i] += 0.5 * next.ay[i] * dt;
            next.vz[i] += 0.5 * next.az[i] * dt;
        }
    });
}
