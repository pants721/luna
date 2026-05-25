#include "solvers/barnes_hut_openmp.hpp"
#include "ephemeris.hpp"
#include "octree.hpp"

void solvers::BarnesHutOpenMP::resetAccel(physics::Ephemeris &s) {
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < s.n; ++i) {
        s.ax[i] = 0.0;
        s.ay[i] = 0.0;
        s.az[i] = 0.0;
    }
}

void solvers::BarnesHutOpenMP::computeBounds(physics::Ephemeris &s) {
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

void solvers::BarnesHutOpenMP::computeAccel(physics::Ephemeris &s) {
    physics::Octree tree = physics::Octree(&s);

    tree.build();
    tree.computeMass();

    resetAccel(s);

    #pragma omp parallel for schedule(guided)
    for (size_t i = 0; i < s.n; ++i) {
        tree.computeAccelIt(i, theta);
    }
}
