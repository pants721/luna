#pragma once

#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_double4x4.hpp>
#include <glm/ext/vector_double3.hpp>
#include <glm/glm.hpp>

struct CameraConfig {
    double rot_speed;
    double move_speed;
    double fov;

    CameraConfig() : 
        rot_speed(1.0),
        move_speed(10.0),
        fov(45.0)
    {}
};

struct Camera {
    glm::vec3 pos;
    glm::vec3 front; // where we lookin
    glm::vec3 up;
    glm::vec3 right;

    double roll; 
    double pitch;
    double yaw;

    CameraConfig config;

    Camera() :
        pos(0.0, 0.0, 5.0),
        front(0.0, 0.0, -1.0),
        up(0.0, 1.0, 0.0),
        yaw(-90.0), 
        pitch(0.0)
    {
        updateVectors();
    }


    glm::mat4 viewMat();
    glm::mat4 projectionMat();

    void updateVectors();
};
