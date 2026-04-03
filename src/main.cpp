#include <GL/gl.h>
#include <stdio.h>
#include <utility>
#include <immintrin.h>

#include "gfx/renderer.hpp"
#include "constants.hpp"
#include "sim_state.hpp"

#define N 1e4

int main() {
    SimState current(N, {1e2, 1e4}, {-400, 400});
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
