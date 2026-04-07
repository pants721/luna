#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <immintrin.h>

#include "camera.hpp"
#include "constants.hpp"
#include "ephemeris.hpp"
#include "renderer.hpp"

#include <GLFW/glfw3.h>

#define DT 0.01

#define WIN_W 1280
#define WIN_H 960
#define WIN_CENTER_X (WIN_W / 2.0f)
#define WIN_CENTER_Y (WIN_H / 2.0f)
#define WIN_TITLE "Luna - Lu(cas)n(body simulator)a"

#define RENDER_SCALE 1.0f

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow *window = glfwCreateWindow(WIN_W, WIN_H, WIN_TITLE, nullptr, nullptr);
    glfwSetWindowAttrib(window, GLFW_FLOATING, GLFW_TRUE);
    glfwMakeContextCurrent(window);

    // glad
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // load extensions
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, WIN_W, WIN_H);

    // renderer
    Renderer renderer(OPENGL);
    renderer.setup();

    Camera cam;

    Ephemeris current(NUM_BODIES, {1e2, 1e4}, {-200, 200});
    Ephemeris next(NUM_BODIES);

    while (!glfwWindowShouldClose(window)) {
        step(current, next, DT);

        // clear screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw your particles here
        renderer.render(current, cam);
        renderer.draw(current, cam);

        // swap buffers
        glfwSwapBuffers(window);

        // process input/events
        glfwPollEvents();
    }

    glfwTerminate();

    return EXIT_SUCCESS;
}
