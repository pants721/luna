#include "solvers/barnes_hut_single_threaded.hpp"
#include "ephemeris.hpp"
#include "octree.hpp"

void solvers::BarnesHutSingleThreaded::resetAccel(physics::Ephemeris &s) {
    for (size_t i = 0; i < s.n; ++i) {
        s.ax[i] = 0.0;
        s.ay[i] = 0.0;
        s.az[i] = 0.0;
    }
}

void solvers::BarnesHutSingleThreaded::computeBounds(physics::Ephemeris &s) {
    double max_x = -std::numeric_limits<double>::infinity();
    double max_y = -std::numeric_limits<double>::infinity();
    double max_z = -std::numeric_limits<double>::infinity();

    double min_x = std::numeric_limits<double>::infinity();
    double min_y = std::numeric_limits<double>::infinity();
    double min_z = std::numeric_limits<double>::infinity();

    for (size_t i = 0; i < s.n; ++i) {
        double x = s.x[i];
        double y = s.y[i];
        double z = s.z[i];

        max_x = std::max(max_x, x);
        max_y = std::max(max_y, y);
        max_z = std::max(max_z, z);

        min_x = std::min(min_x, x);
        min_y = std::min(min_y, y);
        min_z = std::min(min_z, z);
    }

    s.max_x = max_x;
    s.max_y = max_y;
    s.max_z = max_z;

    s.min_x = min_x;
    s.min_y = min_y;
    s.min_z = min_z;
}

void solvers::BarnesHutSingleThreaded::computeAccel(physics::Ephemeris &s) {
    tree.reset();
    tree.eph = &s;
    tree.build();
    tree.computeMass();

    resetAccel(s);

    for (size_t i = 0; i < s.n; ++i) {
        tree.computeAccelIt(i, theta);
    }
}
