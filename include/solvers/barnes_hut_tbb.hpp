#pragma once

#include "physics/ephemeris.hpp"

namespace solvers {

class BarnesHutTBB {
private:   
    double theta = 0.5;

    double max_x = -std::numeric_limits<double>::infinity();
    double max_y = -std::numeric_limits<double>::infinity();
    double max_z = -std::numeric_limits<double>::infinity();

    double min_x = std::numeric_limits<double>::infinity();
    double min_y = std::numeric_limits<double>::infinity();
    double min_z = std::numeric_limits<double>::infinity();
     
    void resetAccel(physics::Ephemeris &s);
public:
    void computeBounds(physics::Ephemeris &s);
    void computeAccel(physics::Ephemeris &s);
};

}
