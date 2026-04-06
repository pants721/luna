#pragma once

#include "camera.hpp"
#include "ephemeris.hpp"
#include "constants.hpp"

#include <glad/glad.h>
#include <string>
#include <vector>

#define VERTEX_SHADER "shaders/vertex.glsl"
#define FRAG_SHADER "shaders/frag.glsl"

enum GraphicsBackend {
    OPENGL = 0,
};

// OPENGL STUFF
struct OpenGLData {
    unsigned int shader_program;
    GLuint vao, vbo;

    void createVertexObjects();
    void loadShadersFromFiles(const std::string vertex_shader_path, const std::string frag_shader_path);
    void loadShaders(const char *vertex_shader_src, const char *frag_shader_src);

    void uploadVertices(std::vector<float> &packed);

};

struct Renderer {
    GraphicsBackend gfx_backend; 
    OpenGLData opengl_data;

    std::vector<float> vertex_buffer;

    Renderer(GraphicsBackend gfx_backend) : gfx_backend(gfx_backend), vertex_buffer(MAX_BODIES * 3) {}
    void setup();
    void render(Ephemeris &world, Camera &cam);
    void draw(Ephemeris &world, Camera &cam);
};
