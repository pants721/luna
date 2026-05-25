#pragma once

#include "ephemeris.hpp"
#include "sim_config.hpp"
#include "step.hpp"
#include <utility>
namespace sim {

template<typename Solver>
class LunaEngine {
protected:
    Solver solver;

    physics::Ephemeris next;
public:
    physics::Ephemeris current;

    LunaEngine(cfg::SimConfig cfg) : current(cfg), next(cfg.num_bodies) {}

    void step(double dt) {
        solver.computeBounds(current);
        solver.computeAccel(current);
        integrate(current, next, dt);
        solver.computeAccel(next);
        halfKick(current, next, dt);
        std::swap(current, next);

    }
};

}
