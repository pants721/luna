#include "cfg/sim_config.hpp"
#include <rfl/json/load.hpp>
#include <rfl/json/read.hpp>
#include <rfl/json/save.hpp>
#include <rfl/json/write.hpp>
#include <string>
#include <rfl/json.hpp>
#include <rfl.hpp>

using namespace cfg;

cfg::SimConfig cfg::SimConfig::load(std::string file_path) {
    auto result = rfl::json::load<SimConfig>(file_path);
    return result.value();
}

bool cfg::SimConfig::save(std::string file_path) {
    rfl::json::save(file_path, this);
    return true;
}
