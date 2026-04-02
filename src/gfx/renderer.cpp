#include "gfx/renderer.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/VertexArray.hpp"
#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/WindowEnums.hpp"
#include "constants.hpp"
#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "sim_state.hpp"
#include <cmath>
#include <cstddef>
#include <numbers>

float scaleRadiusByZ(float z, float scale_factor = 0.0001) {
    float t = std::atan(z * scale_factor) / (std::numbers::pi / 2.0);
    t = (t + 1.0) / 2.0;
    return MIN_BODY_RADIUS + t * (MAX_BODY_RADIUS - MIN_BODY_RADIUS);
}

float scaleRadiusByZLog(float z, float offset = 1.0) {
    float t = std::log(std::abs(z) + offset);
    t = std::tanh(t);  // clamp to [0,1] roughly
    return MIN_BODY_RADIUS + t * (MAX_BODY_RADIUS - MIN_BODY_RADIUS);
}

sf::Color colorFromZ(double z, float scaleFactor = 0.0001f) {
    // t ∈ [0,1] indicates relative depth
    float t = std::atan(z * scaleFactor) / (3.14159265f / 2.0f); // [-1,1]
    t = (t + 1.f) / 2.f; // remap to [0,1]

    int brightness = 50 + t * 205; // 50..255
    return sf::Color(brightness, brightness, brightness);
}

sf::Vector2f project(float sim_a, float sim_b, const sf::Vector2f &origin, float scale) {
    return sf::Vector2f(origin.x + sim_a * scale,
                        origin.y - sim_b * scale); // flip y
}

Renderer::Renderer(unsigned int w, unsigned int h, const char *title, sf::ContextSettings settings) 
    : window(sf::VideoMode({w, h}), title, sf::Style::Default, sf::State::Windowed, settings) {}

void Renderer::draw(const SimState &s) {
    sf::VertexArray points(sf::PrimitiveType::Points, s.n);

    for (size_t i = 0; i < s.n; ++i) {
        sf::Vector2f pos = project(s.x[i], s.y[i], ORIGIN, PIXELS_PER_UNIT);
        points[i].position = pos;
        points[i].color = colorFromZ(s.z[i]);
    }

    window.draw(points);
}
