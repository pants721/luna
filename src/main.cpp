#include <GL/gl.h>
#include <stdio.h>
#include <utility>

#include "gfx/renderer.hpp"
#include "constants.hpp"
#include "sim_state.hpp"

#define N 1e4

int main() {
    SimState current(N, {1e9, 1e11}, {-2e5,2e5});
    SimState next(N);
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    Renderer renderer(SCREEN_W, SCREEN_H, "N-Body", settings);
    glPointSize(2.f);
    sf::Clock clock;

    while (renderer.window.isOpen()) {
        while (const std::optional event = renderer.window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                renderer.window.close();
            }
        }
        float dt = clock.restart().asSeconds();
        if (!renderer.paused) {
            step(current, next, 0.01);
        }
        renderer.window.clear(sf::Color::Black);
        renderer.draw(current);
        renderer.window.display();
    }
}
