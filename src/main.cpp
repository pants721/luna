#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <utility>
#include <immintrin.h>

#include "camera.hpp"
#include "constants.hpp"
#include "ephemeris.hpp"
#include "renderer.hpp"
#include "sim_config.hpp"

#include <GLFW/glfw3.h>

void processInput(GLFWwindow* window, Camera &cam, float delta_time) {
    float move_velocity = cam.config.move_speed * delta_time;
    float rot_velocity = cam.config.rot_speed * delta_time;


    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cam.move(Camera::FORWARD, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cam.move(Camera::BACKWARD, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cam.move(Camera::LEFT, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cam.move(Camera::RIGHT, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        cam.move(Camera::UP, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        cam.move(Camera::DOWN, delta_time);
    }
}

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
    // glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_POINT_SMOOTH);
    // alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, WIN_W, WIN_H);

    // renderer
    Renderer renderer(OPENGL);
    renderer.setup();

    Camera cam;

    // load config
    SimConfig sim_config = load(CONFIG_PATH);

    // set up bodies
    Ephemeris current(sim_config);
    Ephemeris next(sim_config.num_bodies);

    float last_frame = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float current_frame = glfwGetTime();
        float delta_time = current_frame - last_frame;
        last_frame = current_frame;

        processInput(window, cam, delta_time);

        step(current, next, TIME_STEP);

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
