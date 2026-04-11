#include "camera.hpp"
#include "constants.hpp"

#include <cmath>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

glm::mat4 Camera::viewMat() {
    return glm::lookAt(pos, pos + front, up);
}

glm::mat4 Camera::projectionMat() {
    return glm::perspective(
        glm::radians(config.fov),
        (double)WIN_W / (double)WIN_H,
        0.1, 100.0
    );
}

void Camera::updateVectors() {
    glm::vec3 f;

    f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y = sin(glm::radians(pitch));
    f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(f);
    right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    up    = glm::normalize(glm::cross(right, front));
}
