#pragma once

#include "constants.hpp"
#include "physics/octree.hpp"
#include "physics/ephemeris.hpp"

namespace solvers {

class BarnesHutOpenMP {
private:
    double theta = BH_THETA;

    double max_x = -std::numeric_limits<double>::infinity();
    double max_y = -std::numeric_limits<double>::infinity();
    double max_z = -std::numeric_limits<double>::infinity();

    double min_x = std::numeric_limits<double>::infinity();
    double min_y = std::numeric_limits<double>::infinity();
    double min_z = std::numeric_limits<double>::infinity();
     
    physics::Octree tree;

    void resetAccel(physics::Ephemeris &s);
public:
    void computeBounds(physics::Ephemeris &s);
    void computeAccel(physics::Ephemeris &s);
};

}
