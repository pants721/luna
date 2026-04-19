#pragma once

#include <string>

constexpr int MAX_BODIES = 1e6;

constexpr double SOFTENING = 1e-12;
constexpr double BH_THETA = 1.0;
// TODO: probably make this a part of Ephemeris so that we can like pause and speed up
constexpr double TIME_STEP = 0.01;

// Units

// Gravitational constant
constexpr double G = 1.0;

// Length reference unit
constexpr double L0 = 1.0;

// Mass reference unit
constexpr double M0 = 1.0;

// Time reference unit
constexpr double T0 = 1.0;

constexpr float WIN_W = 1280;
constexpr float WIN_H = 960;
constexpr float WIN_CENTER_X = (WIN_W / 2.0f);
constexpr float WIN_CENTER_Y = (WIN_H / 2.0f);
constexpr auto WIN_TITLE = "Luna - LUcas N-body Analyzer";

constexpr auto DEFAULT_CONFIG_PATH = "config.json";
