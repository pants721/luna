#include "solvers/direct_openmp.hpp"
#include "constants.hpp"
#include "ephemeris.hpp"

void solvers::DirectOpenMP::computeBounds(physics::Ephemeris &s) {}

void solvers::DirectOpenMP::computeAccel(physics::Ephemeris &s) {
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

            double dist_sq = (dx * dx + dy * dy + dz * dz) + softening;

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
