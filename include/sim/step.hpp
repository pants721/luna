#pragma once

#include "ephemeris.hpp"
namespace sim {

void resetAccel(physics::Ephemeris &state);
void computeBounds(physics::Ephemeris &s);
void computeAccelDirect(physics::Ephemeris &state);
void computeAccelBH(physics::Ephemeris &state);
void integrate(physics::Ephemeris &current, physics::Ephemeris &next, double dt);
void halfKick(physics::Ephemeris &current, physics::Ephemeris &next, double dt);

// Force policy
namespace force_policy {

struct Direct {
    static void step(physics::Ephemeris &current, physics::Ephemeris &next, double dt);
};

struct BarnesHut {
    static void step(physics::Ephemeris &current, physics::Ephemeris &next, double dt);
};

}

template <typename Policy>
void step(physics::Ephemeris &current, physics::Ephemeris &next, double dt) {
    Policy::step(current, next, dt);
}

}
