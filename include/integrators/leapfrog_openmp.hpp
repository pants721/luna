#pragma once

#include "physics/ephemeris.hpp"

namespace integrators {

class LeapFrogOpenMP {
public:
    void preForceUpdate(physics::Ephemeris &current, 
                        physics::Ephemeris &next, double dt);
    void postForceUpdate(physics::Ephemeris &current, 
                        physics::Ephemeris &next, double dt);
};

}
