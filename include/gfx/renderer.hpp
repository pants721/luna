#pragma once

#include "gfx/camera.hpp"
#include "physics/ephemeris.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#define VERTEX_SHADER "shaders/vertex.glsl"
#define FRAG_SHADER "shaders/frag.glsl"

// enum GraphicsBackend {
//     OPENGL = 0,
// };

// OPENGL STUFF
struct OpenGLData {
    unsigned int shader_program;
    GLuint vao, vbo;
    float *vbo_buffer;

    GLFWwindow *window;

    void createVertexObjects();
    void loadShadersFromFiles(const std::string vertex_shader_path, const std::string frag_shader_path);
    void loadShaders(const char *vertex_shader_src, const char *frag_shader_src);

    void uploadVertices(std::vector<float> &packed);

};

struct Renderer {
    OpenGLData opengl_data;

    void setup();
    void clear();
    void render(physics::Ephemeris &world, Camera &cam);
    void draw(physics::Ephemeris &world, Camera &cam);
};
