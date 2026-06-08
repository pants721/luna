#include "gfx/renderer.hpp"
#include "constants.hpp"
#include "camera.hpp"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <string>

using namespace gfx;

static std::string readFile(const std::string &path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "Cannot open shader: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static GLuint compileShader(GLenum type, const char *src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    int ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << "\n";
    }
    return shader;
}

GLuint OpenGLData::buildProgram(const std::string &vert_path, const std::string &frag_path) {
    std::string vs_src = readFile(vert_path);
    std::string fs_src = readFile(frag_path);
    GLuint vert = compileShader(GL_VERTEX_SHADER,   vs_src.c_str());
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fs_src.c_str());
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    int ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(prog, 512, nullptr, log);
        std::cerr << "Program link error:\n" << log << "\n";
    }
    glDeleteShader(vert);
    glDeleteShader(frag);
    return prog;
}

void OpenGLData::createParticleProgram() {
    particle_program = buildProgram(PARTICLE_VERT, PARTICLE_FRAG);
    loc_view  = glGetUniformLocation(particle_program, "uView");
    loc_proj  = glGetUniformLocation(particle_program, "uProjection");
    loc_model = glGetUniformLocation(particle_program, "uModel");
}

void OpenGLData::createBlurProgram() {
    blur_program        = buildProgram(SCREEN_VERT, BLUR_FRAG);
    loc_blur_image      = glGetUniformLocation(blur_program, "uImage");
    loc_blur_horizontal = glGetUniformLocation(blur_program, "uHorizontal");
}

void OpenGLData::createCompositeProgram() {
    composite_program = buildProgram(SCREEN_VERT, COMPOSITE_FRAG);
    loc_comp_scene    = glGetUniformLocation(composite_program, "uScene");
    loc_comp_bloom    = glGetUniformLocation(composite_program, "uBloom");
}

void OpenGLData::createVertexObjects() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_BODIES * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    cpu_buffer.resize(MAX_BODIES * 3);
}

void OpenGLData::createQuad() {
    // Fullscreen triangle pair in NDC with UV coords
    static const float quad[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
    };
    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

static GLuint makeHDRTexture(int w, int h) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}

void OpenGLData::createFramebuffers() {
    const int W = static_cast<int>(WIN_W);
    const int H = static_cast<int>(WIN_H);

    // HDR scene FBO
    glGenFramebuffers(1, &hdr_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);
    hdr_texture = makeHDRTexture(W, H);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdr_texture, 0);

    // Ping-pong FBOs for separable blur
    glGenFramebuffers(2, pingpong_fbo);
    for (int i = 0; i < 2; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpong_fbo[i]);
        pingpong_texture[i] = makeHDRTexture(W, H);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpong_texture[i], 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::setup() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    opengl_data.window = glfwCreateWindow(WIN_W, WIN_H, WIN_TITLE, nullptr, nullptr);
    glfwSetWindowAttrib(opengl_data.window, GLFW_FLOATING, GLFW_TRUE);
    glfwMakeContextCurrent(opengl_data.window);
    glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glViewport(0, 0, WIN_W, WIN_H);

    opengl_data.createParticleProgram();
    opengl_data.createBlurProgram();
    opengl_data.createCompositeProgram();
    opengl_data.createVertexObjects();
    opengl_data.createQuad();
    opengl_data.createFramebuffers();
}

void Renderer::clear() {
    // draw() manages its own framebuffer clearing
}

void Renderer::render(physics::Ephemeris &world, Camera &cam) {
    for (int i = 0; i < (int)world.n; i++) {
        opengl_data.cpu_buffer[i * 3 + 0] = static_cast<float>(world.x[i] / 200.0);
        opengl_data.cpu_buffer[i * 3 + 1] = static_cast<float>(world.y[i] / 200.0);
        opengl_data.cpu_buffer[i * 3 + 2] = static_cast<float>(world.z[i] / 200.0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, opengl_data.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, world.n * 3 * sizeof(float), opengl_data.cpu_buffer.data());
}

void Renderer::draw(physics::Ephemeris &world, Camera &cam) {
    glBindFramebuffer(GL_FRAMEBUFFER, opengl_data.hdr_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);  // pure additive — alpha channel plays no role

    glUseProgram(opengl_data.particle_program);
    glUniformMatrix4fv(opengl_data.loc_view,  1, GL_FALSE, glm::value_ptr(cam.viewMat()));
    glUniformMatrix4fv(opengl_data.loc_proj,  1, GL_FALSE, glm::value_ptr(cam.projectionMat()));
    glUniformMatrix4fv(opengl_data.loc_model, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

    glBindVertexArray(opengl_data.vao);
    glDrawArrays(GL_POINTS, 0, world.n);

    glDisable(GL_BLEND);
    glUseProgram(opengl_data.blur_program);
    glBindVertexArray(opengl_data.quad_vao);

    // Clear both ping-pong buffers so no stale data from prior frames leaks in
    for (int i = 0; i < 2; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, opengl_data.pingpong_fbo[i]);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    const int N_BLUR_PASSES = 5;
    bool horizontal = true;
    for (int i = 0; i < N_BLUR_PASSES; ++i) {
        int write_buf = horizontal ? 0 : 1;
        int read_buf  = horizontal ? 1 : 0;

        glBindFramebuffer(GL_FRAMEBUFFER, opengl_data.pingpong_fbo[write_buf]);
        glUniform1i(opengl_data.loc_blur_horizontal, horizontal ? 1 : 0);

        glActiveTexture(GL_TEXTURE0);
        GLuint src = (i == 0) ? opengl_data.hdr_texture
                               : opengl_data.pingpong_texture[read_buf];
        glBindTexture(GL_TEXTURE_2D, src);
        glUniform1i(opengl_data.loc_blur_image, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        horizontal = !horizontal;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(opengl_data.composite_program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl_data.hdr_texture);
    glUniform1i(opengl_data.loc_comp_scene, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, opengl_data.pingpong_texture[1]);
    glUniform1i(opengl_data.loc_comp_bloom, 1);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}
