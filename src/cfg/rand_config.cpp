#include "cfg/rand_config.hpp"

using namespace cfg;

std::pair<double, double> cfg::RandConfig::x_range() {
    return {pos_range.first.x, pos_range.second.x};
}

std::pair<double, double> cfg::RandConfig::y_range() {
    return {pos_range.first.y, pos_range.second.y};
}

std::pair<double, double> cfg::RandConfig::z_range() {
    return {pos_range.first.z, pos_range.second.z};
}

