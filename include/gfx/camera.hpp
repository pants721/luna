#pragma once

struct CameraConfig {
    double rot_speed = 1.0;
    double move_speed = 1.0;
    double fov = 1.0;
};

struct Camera {
    double x = 0.0, y = 0.0, z = 0.0;
    double roll = 0.0, pitch = 0.0, yaw = 0.0;
    CameraConfig config;
};
