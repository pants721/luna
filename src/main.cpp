#include <GL/gl.h>
#include <stdio.h>

#include "gfx/renderer.hpp"
#include "constants.hpp"
#include "sim_state.hpp"

int main() {
    SimState state(1e4, {1e9, 1e11}, {-2e5,2e5});
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    Renderer renderer(SCREEN_W, SCREEN_H, "N-Body", settings);
    glPointSize(5.f);
    sf::Clock clock;

    while (renderer.window.isOpen()) {
        while (const std::optional event = renderer.window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                renderer.window.close();
            }
        }
        float dt = clock.restart().asSeconds();
        if (!renderer.paused) {
            step(state, 0.1);
        }
        renderer.window.clear(sf::Color::Black);
        renderer.draw(state);
        renderer.window.display();
    }
}
