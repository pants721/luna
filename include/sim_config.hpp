#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

struct Position {
    double x;
    double y;
    double z;
};

struct BodyInfo {
    double mass; 
    Position initial_pos;
};

struct RandConfig {
    std::pair<double, double> mass_range;
    std::pair<Position, Position> pos_range;

    std::pair<double, double> x_range();
    std::pair<double, double> y_range();
    std::pair<double, double> z_range();
};

struct SimConfig {
    int num_bodies;
    std::optional<std::vector<BodyInfo>> bodies;
    std::optional<RandConfig> random_config;
};

SimConfig load(std::string file_path);
bool save(SimConfig *sim_config, std::string file_path);

