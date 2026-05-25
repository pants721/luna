#include "direct_tbb.hpp"
#include "ephemeris.hpp"

#include <tbb/tbb.h>

void solvers::DirectTBB::computeBounds(physics::Ephemeris &s) {}

void solvers::DirectTBB::computeAccel(physics::Ephemeris &s) {
    size_t grain_size = 64;

    tbb::parallel_for(tbb::blocked_range<size_t>(0, s.n, grain_size), 
    [&](const tbb::blocked_range<size_t>& range) {
        
        for (size_t i = range.begin(); i != range.end(); ++i) {
            double ax = 0.0;
            double ay = 0.0;
            double az = 0.0;

            double xi = s.x[i];
            double yi = s.y[i];
            double zi = s.z[i];

            for (size_t j = 0; j < s.n; ++j) {
                double xj = s.x[j];
                double yj = s.y[j];
                double zj = s.z[j];
                double mj = s.mass[j];

                double dx = xj - xi;
                double dy = yj - yi;
                double dz = zj - zi;

                double dist_sq = (dx * dx + dy * dy + dz * dz) + softening;

                double r_inv = 1.0 / std::sqrt(dist_sq);
                double r_inv_sq = r_inv * r_inv;
                double r_inv_cub = r_inv_sq * r_inv;

                double mask = static_cast<double>(i != j);
                double common = G * mj * r_inv_cub * mask;
                
                ax += dx * common;
                ay += dy * common;
                az += dz * common;
            }

            s.ax[i] = ax;
            s.ay[i] = ay;
            s.az[i] = az;
        }
    });
}
