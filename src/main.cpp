#include <stdio.h>

#include "sim_state.hpp"

int main(int argc, char **argv) {
    SimState state(50, {0.001, 1.0}, {0.0, 15.0}, {0.0, 15.0}, {0.0, 15.0});

    compute(state);

    double dt = 0.01;
    int steps = 5000;

    for (int i = 0; i < steps; ++i) {
        step(state, dt);
    }

    return 0;
}
