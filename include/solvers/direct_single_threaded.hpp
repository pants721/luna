#pragma once

#include "constants.hpp"
#include "physics/ephemeris.hpp"

namespace solvers {

class DirectSingleThreaded {
private:
    double softening = SOFTENING;
public:
    void computeBounds(physics::Ephemeris &s);
    void computeAccel(physics::Ephemeris &s);
};

}
