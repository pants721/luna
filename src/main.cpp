#include <cstdlib>
#include <immintrin.h>

#include "camera.hpp"
#include "constants.hpp"
#include "physics/ephemeris.hpp"
#include "gfx/renderer.hpp"
#include "cfg/sim_config.hpp"

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
    // renderer
    Renderer renderer;
    renderer.setup();

    Camera cam;

    // load config
    cfg::SimConfig sim_config = cfg::SimConfig::load(DEFAULT_CONFIG_PATH);

    // set up bodies
    Ephemeris current(sim_config);
    Ephemeris next(sim_config.num_bodies);

    float last_frame = glfwGetTime();

    while (!glfwWindowShouldClose(renderer.opengl_data.window)) {
        float current_frame = glfwGetTime();
        float delta_time = current_frame - last_frame;
        last_frame = current_frame;

        processInput(renderer.opengl_data.window, cam, delta_time);

        step(current, next, TIME_STEP);

        // clear screen
        renderer.clear();

        // draw particles
        renderer.render(current, cam);
        renderer.draw(current, cam);

        // swap buffers
        glfwSwapBuffers(renderer.opengl_data.window);

        // process input/events
        glfwPollEvents();
    }

    glfwTerminate();

    return EXIT_SUCCESS;
}
