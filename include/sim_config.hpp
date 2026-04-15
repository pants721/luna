#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

struct Vec3 {
    double x;
    double y;
    double z;
};

struct BodyInfo {
    double mass; 
    Vec3 initial_pos;
    std::optional<Vec3> initial_vel;
};

struct RandConfig {
    std::pair<double, double> mass_range;
    std::pair<Vec3, Vec3> pos_range;

    std::pair<double, double> x_range();
    std::pair<double, double> y_range();
    std::pair<double, double> z_range();
};

struct SimConfig {
    int num_bodies;
    std::optional<std::vector<BodyInfo>> bodies;
    std::optional<RandConfig> random_config;

    static SimConfig load(std::string file_path);
    bool save(std::string file_path);
};


