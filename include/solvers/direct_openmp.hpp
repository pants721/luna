#pragma once

#include "physics/ephemeris.hpp"

namespace solvers {

class DirectOpenMP {
private:
   double softening = 1e-12;
public:
    void computeBounds(physics::Ephemeris &s);
    void computeAccel(physics::Ephemeris &s);
};

}
