#include "camera.hpp"
#include "constants.hpp"

#include <cmath>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

constexpr double NEAR_PLANE = 0.1;
constexpr double FAR_PLANE = 1000.0;

glm::mat4 gfx::Camera::viewMat() {
    return glm::lookAt(pos, target, up);
}

glm::mat4 gfx::Camera::projectionMat() {
    return glm::perspective(
        glm::radians(config.fov),
        (double)WIN_W / (double)WIN_H,
        NEAR_PLANE, FAR_PLANE
    );
}

void gfx::Camera::updateVectors() {
    front = glm::normalize(target - pos);
    right = glm::normalize(glm::cross(front, up));
    up = glm::normalize(glm::cross(right, front));

    // pos.x = target.x + orbit_radius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    // pos.y = target.y + orbit_radius * sin(glm::radians(pitch));
    // pos.z = target.z + orbit_radius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    //
    // front = glm::normalize(target - pos);
    // right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    // up    = glm::normalize(glm::cross(right, front));

    // glm::vec3 f;
    //
    // f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    // f.y = sin(glm::radians(pitch));
    // f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    // front = glm::normalize(f);
    // right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    // up    = glm::normalize(glm::cross(right, front));
}

void gfx::Camera::move(Camera::Direction dir, float delta_time) {
    float speed = config.move_speed * delta_time;

    // Get the vector from the target to the current camera position
    glm::vec3 rel_pos = pos - target;

    // adjust orbit_radius
    if (dir == FORWARD)  {
        orbit_radius -= speed;
    }
    if (dir == BACKWARD) {
        orbit_radius += speed;
    }

    // make sure we dont go through center
    if (orbit_radius < NEAR_PLANE) {
        orbit_radius = NEAR_PLANE;
    }

    // handle rotations
    float rot_speed = speed * config.rot_speed; 

    glm::mat4 rotation(1.0f);

    // rotate around cams up vector
    if (dir == LEFT) {
        rotation = glm::rotate(glm::mat4(1.0f), -rot_speed, up);
        rel_pos = glm::vec3(rotation * glm::vec4(rel_pos, 1.0f));
    }
    if (dir == RIGHT) {
        rotation = glm::rotate(glm::mat4(1.0f), rot_speed, up);
        rel_pos = glm::vec3(rotation * glm::vec4(rel_pos, 1.0f));
    }

    // rotate around cams right vector
    if (dir == UP) {
        rotation = glm::rotate(glm::mat4(1.0f), -rot_speed, right);
        rel_pos = glm::vec3(rotation * glm::vec4(rel_pos, 1.0f));
    }
    if (dir == DOWN) {
        rotation = glm::rotate(glm::mat4(1.0f), rot_speed, right);
        rel_pos = glm::vec3(rotation * glm::vec4(rel_pos, 1.0f));
    }

    //  update position based on the rotation and current radius
    pos = target + glm::normalize(rel_pos) * orbit_radius;

    updateVectors();
}
