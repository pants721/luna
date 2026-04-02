#include <stdio.h>
#include <GLFW/glfw3.h>

#include "sim_state.hpp"

#define DT 0.001

int main(int argc, char **argv) {
    SimState state(50, {0.001, 1.0}, {0.0, 15.0}, {0.0, 15.0}, {0.0, 15.0});

    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glEnable(GL_DEPTH_TEST);


    GLFWwindow* window = glfwCreateWindow(800, 600, "N-Body Visualization", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    const double fixed_dt = 0.01; // e.g., 100 updates per second
    double accumulator = 0.0;
    double lastFrame = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double currentFrame = glfwGetTime();
        double frameTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        accumulator += frameTime;

        // Update physics in fixed increments
        while (accumulator >= fixed_dt) {
            step(state, fixed_dt);
            accumulator -= fixed_dt;
        }


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;

    return 0;
}
