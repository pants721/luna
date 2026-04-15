#pragma once

#include "cfg/body_info.hpp"
#include "cfg/rand_config.hpp"

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace cfg {

struct SimConfig {
    int num_bodies;
    std::optional<std::vector<BodyInfo>> bodies;
    std::optional<RandConfig> random_config;

    static SimConfig load(std::string file_path);
    bool save(std::string file_path);
};

}

