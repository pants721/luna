#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <utility>
#include <immintrin.h>
#include <GL/gl.h>
#include <SFML/Window.hpp>

#include "constants.hpp"
#include "sim_state.hpp"

#define N 10000
#define DT 0.01

#define WIN_W 1280
#define WIN_H 960
#define WIN_CENTER_X (WIN_W / 2.0f)
#define WIN_CENTER_Y (WIN_H / 2.0f)
#define WIN_TITLE "Luna - Lu(cas)n(body simulator)a"

#define RENDER_SCALE 1.0f

void render(SimState &state, sf::RenderWindow &window) {
    sf::VertexArray points(sf::PrimitiveType::Points, state.n);

    for (int i = 0; i < state.n; ++i) {
        float px = (state.x[i] * RENDER_SCALE) + WIN_CENTER_X;
        float py = (state.y[i] * RENDER_SCALE) + WIN_CENTER_Y;
        points[i].position = sf::Vector2f(px, py);

        float speed = std::sqrt(
            state.vx[i] * state.vx[i] + 
            state.vy[i] * state.vy[i] + 
            state.vz[i] * state.vz[i]
        );


        points[i].color = sf::Color(189, 79, 55);
    }

    window.draw(points);
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WIN_W, WIN_H}), WIN_TITLE);
    window.setFramerateLimit(60);
    glPointSize(2.0f);

    SimState current(N, {1e2, 1e4}, {-400, 400});
    SimState next(N);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        step(current, next, DT);

        // sf::RectangleShape fade(sf::Vector2f(WIN_W, WIN_H));
        // fade.setFillColor(sf::Color(0, 0, 0, 10));
        // window.draw(fade);

        window.clear();


        render(current, window);

        window.display();
    }

    return 0;
}
