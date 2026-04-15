#pragma once

#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_double4x4.hpp>
#include <glm/ext/vector_double3.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>

namespace gfx {

struct CameraConfig {
    double rot_speed;
    double move_speed;
    double fov;

    CameraConfig() : 
        rot_speed(0.5),
        move_speed(10.0),
        fov(45.0)
    {}
};

struct Camera {
    float orbit_radius;

    glm::vec3 pos;
    glm::vec3 front; // where we lookin
    glm::vec3 up;
    glm::vec3 right;

    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);

    // double roll; 
    // double pitch;
    // double yaw;

    CameraConfig config;

    enum Direction {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN,
    };

    Camera() :
        orbit_radius(10.0f),
        front(0.0f, 0.0f, -1.0f),
        up(0.0f, 1.0f, 0.0f),
        target(0.0f, 0.0f, 0.0f)
    {
        pos = target + glm::vec3(0.0f, 0.0f, orbit_radius);
        updateVectors();
    }


    glm::mat4 viewMat();
    glm::mat4 projectionMat();

    void updateVectors();

    void move(Direction dir, float delta_time);
};

}
