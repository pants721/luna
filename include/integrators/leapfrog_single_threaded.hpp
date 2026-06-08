#pragma once

#include "physics/ephemeris.hpp"

namespace integrators {

class LeapFrogSingleThreaded {
public:
    void preForceUpdate(physics::Ephemeris &current,
                        physics::Ephemeris &next, double dt);
    void postForceUpdate(physics::Ephemeris &current,
                        physics::Ephemeris &next, double dt);
};

}
