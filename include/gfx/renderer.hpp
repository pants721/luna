#pragma once

#include "SFML/Graphics/RenderWindow.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "gfx/camera.hpp"
#include "sim_state.hpp"

struct Renderer {
    sf::RenderWindow window;

    bool paused = false;
    bool mouseCaptured = true;

    Renderer(unsigned int w, unsigned int h, const char *title, sf::ContextSettings settings);
    void draw(const SimState &s);
    void render(const SimState &s);
};
