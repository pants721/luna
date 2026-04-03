#include <utility>
#include <immintrin.h>

#include "constants.hpp"
#include "sim_state.hpp"

#define N 1e4
#define DT 0.01
#define RUNS 5000

int main() {
    SimState current(N, {1e2, 1e4}, {-400, 400});
    SimState next(N);

    for (int i = 0; i < RUNS; i++) {
        step(current, next, DT);
    }
}
