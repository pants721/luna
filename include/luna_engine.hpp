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
    bool first_step = true;

    physics::Ephemeris next;
public:
    physics::Ephemeris current;

    LunaEngine(cfg::SimConfig cfg) : current(cfg), next(cfg.num_bodies) {}

    void step(double dt) {
        solver.computeBounds(current);
        // a(x_n) was already computed as a(x_{n+1}) at the end of the previous
        // step — skip the redundant force evaluation every step after the first.
        if (first_step) {
            solver.computeAccel(current);
            first_step = false;
        }
        integrator.preForceUpdate(current, next, dt);
        solver.computeBounds(next);
        solver.computeAccel(next);
        integrator.postForceUpdate(current, next, dt);
        std::swap(current, next);
    }
};

}
