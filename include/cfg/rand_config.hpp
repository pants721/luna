#pragma once
#include "cfg/common.hpp"

#include <utility>

namespace cfg {

struct RandConfig {
    std::pair<double, double> mass_range;
    std::pair<Vec3, Vec3> pos_range;

    std::pair<double, double> x_range();
    std::pair<double, double> y_range();
    std::pair<double, double> z_range();
};

}
