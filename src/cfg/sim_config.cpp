#include "cfg/sim_config.hpp"
#include "common.hpp"
#include "rand_config.hpp"
#include <optional>
#include <rfl/json/load.hpp>
#include <rfl/json/read.hpp>
#include <rfl/json/save.hpp>
#include <rfl/json/write.hpp>
#include <string>
#include <rfl/json.hpp>
#include <rfl.hpp>
#include <vector>

cfg::SimConfig cfg::SimConfig::default_cfg() {
    return SimConfig {
        .num_bodies =  1000,
        .bodies = std::nullopt,
        .random_config = RandConfig {
            .mass_range = {100.0, 10000.0},
            .pos_range = {
                Vec3(-200.0, -200.0, -200.0),
                Vec3(200.0, 200.0, 200.0),
            }
        },
    };
}

cfg::SimConfig cfg::SimConfig::load(std::string file_path) {
    auto result = rfl::json::load<SimConfig>(file_path);
    return result.value();
}

bool cfg::SimConfig::save(std::string file_path) {
    rfl::json::save(file_path, this);
    return true;
}
