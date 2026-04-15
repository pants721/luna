#include "sim_config.hpp"
#include <rfl/json/load.hpp>
#include <rfl/json/read.hpp>
#include <rfl/json/save.hpp>
#include <rfl/json/write.hpp>
#include <string>
#include <rfl/json.hpp>
#include <rfl.hpp>
#include <utility>

std::pair<double, double> RandConfig::x_range() {
    return {pos_range.first.x, pos_range.second.x};
}

std::pair<double, double> RandConfig::y_range() {
    return {pos_range.first.y, pos_range.second.y};
}

std::pair<double, double> RandConfig::z_range() {
    return {pos_range.first.z, pos_range.second.z};
}

SimConfig SimConfig::load(std::string file_path) {
    auto result = rfl::json::load<SimConfig>(file_path);
    return result.value();
}

bool SimConfig::save(std::string file_path) {
    rfl::json::save(file_path, this);
    return true;
}
