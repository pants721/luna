#pragma once

constexpr int NUM_BODIES = 1e3;
constexpr int MAX_BODIES = 1e6;

constexpr double SOFTENING = 1e-12;

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
// constexpr sf::Color BODY_COLOR = sf::Color::White;
// constexpr sf::Vector2f ORIGIN = {SCREEN_W / 2.0, SCREEN_H / 2.0};

