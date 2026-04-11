#pragma once

constexpr int NUM_BODIES = 1e3;
constexpr int MAX_BODIES = 1e6;

constexpr double SOFTENING = 1e-12;
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

// GFX
constexpr float PIXELS_PER_UNIT = 1.0 / L0;
constexpr float MIN_BODY_RADIUS = 0.1;
constexpr float BODY_RADIUS = 1.5;
constexpr float MAX_BODY_RADIUS = 3.0;

constexpr float WIN_W = 1280;
constexpr float WIN_H = 960;
constexpr float WIN_CENTER_X = (WIN_W / 2.0f);
constexpr float WIN_CENTER_Y = (WIN_H / 2.0f);
constexpr auto WIN_TITLE = "Luna - Lu(cas)n(body simulator)a";

constexpr float RENDER_SCALE = 1.0f;
