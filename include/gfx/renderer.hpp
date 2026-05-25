#pragma once

#include "gfx/camera.hpp"
#include "physics/ephemeris.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

#define PARTICLE_VERT  "shaders/particle.vert"
#define PARTICLE_FRAG  "shaders/particle.frag"
#define SCREEN_VERT    "shaders/screen.vert"
#define BLUR_FRAG      "shaders/blur.frag"
#define COMPOSITE_FRAG "shaders/composite.frag"

namespace gfx {

struct OpenGLData {
    GLFWwindow *window;

    // Particle pass
    GLuint particle_program;
    GLuint vao, vbo;
    std::vector<float> cpu_buffer;
    GLint loc_view, loc_proj, loc_model;

    // Blur pass
    GLuint blur_program;
    GLint loc_blur_image, loc_blur_horizontal;

    // Composite pass
    GLuint composite_program;
    GLint loc_comp_scene, loc_comp_bloom;

    // Fullscreen quad
    GLuint quad_vao, quad_vbo;

    // Framebuffers
    GLuint hdr_fbo, hdr_texture;
    GLuint pingpong_fbo[2], pingpong_texture[2];

    GLuint buildProgram(const std::string &vert_path, const std::string &frag_path);
    void createParticleProgram();
    void createBlurProgram();
    void createCompositeProgram();
    void createVertexObjects();
    void createQuad();
    void createFramebuffers();
};

struct Renderer {
    OpenGLData opengl_data;

    void setup();
    void clear();
    void render(physics::Ephemeris &world, Camera &cam);
    void draw(physics::Ephemeris &world, Camera &cam);
};

}
