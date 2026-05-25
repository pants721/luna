#pragma once

#include "ephemeris.hpp"
#include "sim_config.hpp"
#include <utility>
namespace sim {

template<typename Solver, typename Integrator>
class LunaEngine {
protected:
    Solver solver;
    Integrator integrator;

    physics::Ephemeris next;
public:
    physics::Ephemeris current;

    LunaEngine(cfg::SimConfig cfg) : current(cfg), next(cfg.num_bodies) {}

    void step(double dt) {
        solver.computeBounds(current);
        solver.computeAccel(current);
        integrator.preForceUpdate(current, next, dt);
        solver.computeAccel(next);
        integrator.postForceUpdate(current, next, dt);
        std::swap(current, next);
    }
};

}
