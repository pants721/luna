#pragma once

#include "cfg/common.hpp"

#include <optional>

namespace cfg {

struct BodyInfo {
    double mass; 
    Vec3 initial_pos;
    std::optional<Vec3> initial_vel;
};

}
