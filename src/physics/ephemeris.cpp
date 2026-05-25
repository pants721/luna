#include "physics/ephemeris.hpp"
#include "constants.hpp"
#include "cfg/sim_config.hpp"
#include "octree.hpp"

#include <algorithm>
#include <execution>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <random>
#include <cmath>
#include <utility>
#include <vector>

physics::Ephemeris::Ephemeris(size_t n, std::pair<double, double> mass_range,
                   std::pair<double, double> pos_range) : physics::Ephemeris(n) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<> mass_distr(mass_range.first, mass_range.second);
    std::uniform_real_distribution<> pos_distr(pos_range.first, pos_range.second);

    for (size_t i = 0; i < n; ++i) {
        mass[i] = mass_distr(gen);
        x[i] = pos_distr(gen);
        y[i] = pos_distr(gen);
        z[i] = pos_distr(gen);
    }
}

physics::Ephemeris::Ephemeris(size_t n, std::pair<double, double> mass_range, 
         std::pair<double, double> x_range, 
         std::pair<double, double> y_range,
         std::pair<double, double> z_range) : physics::Ephemeris(n) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<> mass_distr(mass_range.first, mass_range.second);
    std::uniform_real_distribution<> x_distr(x_range.first, x_range.second);
    std::uniform_real_distribution<> y_distr(y_range.first, y_range.second);
    std::uniform_real_distribution<> z_distr(z_range.first, z_range.second);

    for (size_t i = 0; i < n; ++i) {
        mass[i] = mass_distr(gen);
        x[i] = x_distr(gen);
        y[i] = y_distr(gen);
        z[i] = z_distr(gen);
    }
}

physics::Ephemeris::Ephemeris(cfg::SimConfig config) : physics::Ephemeris(config.num_bodies) {
    using namespace cfg;

    n = config.num_bodies;

    // preconfigured bodies
    size_t num_bodies_given = 0;
    if (config.bodies.has_value()) {
        num_bodies_given = config.bodies->size();
        std::vector<BodyInfo> bodies = config.bodies.value();    

        for (size_t i = 0; i < bodies.size(); ++i) {
            BodyInfo body_info = bodies[i];
            mass[i] = body_info.mass;
            x[i] = body_info.initial_pos.x;
            y[i] = body_info.initial_pos.y;
            z[i] = body_info.initial_pos.z;

            if (body_info.initial_vel.has_value()) {
                auto initial_vel = body_info.initial_vel.value();
                vx[i] = initial_vel.x;
                vy[i] = initial_vel.y;
                vz[i] = initial_vel.z;
            }
        }
    }


    // random config
    if (config.random_config.has_value()) {
        RandConfig random_config = config.random_config.value();
        std::pair<double, double> mass_range = random_config.mass_range;
        std::pair<double, double> x_range = random_config.x_range();
        std::pair<double, double> y_range = random_config.y_range();
        std::pair<double, double> z_range = random_config.z_range();

        std::random_device rd;
        std::mt19937 gen(rd());

        std::uniform_real_distribution<> mass_distr(mass_range.first, mass_range.second);
        std::uniform_real_distribution<> x_distr(x_range.first, x_range.second);
        std::uniform_real_distribution<> y_distr(y_range.first, y_range.second);
        std::uniform_real_distribution<> z_distr(z_range.first, z_range.second);

        // randomly generate n bodies that havent been given yet
        for (size_t i = num_bodies_given; i < n; ++i) {
            mass[i] = mass_distr(gen);
            x[i] = x_distr(gen);
            y[i] = y_distr(gen);
            z[i] = z_distr(gen);
        }
    } else if (num_bodies_given != n) { // no random config and not enough bodies given so shrink n
        n = num_bodies_given;
    }
}

void physics::printState(physics::Ephemeris &state, int step) {
    for (size_t i = 0; i < state.n; ++i) {
        printf("%d,%zu,%f,%f,%f\n", step, i, state.x[i], state.y[i], state.z[i]);
    }
}
