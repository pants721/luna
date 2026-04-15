#include "gfx/renderer.hpp"
#include "constants.hpp"
#include "camera.hpp"

#include <cstdlib>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace gfx;

void gfx::Renderer::setup() {
    // setup window
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    opengl_data.window = glfwCreateWindow(WIN_W, WIN_H, WIN_TITLE, nullptr, nullptr);
    glfwSetWindowAttrib(opengl_data.window, GLFW_FLOATING, GLFW_TRUE);
    glfwMakeContextCurrent(opengl_data.window);

    // glad
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // load extensions
    glEnable(GL_PROGRAM_POINT_SIZE);
    // glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    // alpha blending
    glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);

    glViewport(0, 0, WIN_W, WIN_H);

    glEnable(GL_PROGRAM_POINT_SIZE);
    opengl_data.loadShadersFromFiles(VERTEX_SHADER, FRAG_SHADER);
    opengl_data.createVertexObjects();
}

void gfx::Renderer::clear() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gfx::Renderer::render(physics::Ephemeris &world, Camera &cam) {
    for (int i = 0; i < world.n; i++) {
        opengl_data.vbo_buffer[i * 3 + 0] = static_cast<float>(world.x[i] / 200);
        opengl_data.vbo_buffer[i * 3 + 1] = static_cast<float>(world.y[i] / 200);
        opengl_data.vbo_buffer[i * 3 + 2] = static_cast<float>(world.z[i] / 200);
    }
}

void gfx::Renderer::draw(physics::Ephemeris &world, Camera &cam) {
    glUseProgram(opengl_data.shader_program);

    glm::mat4 cam_view = cam.viewMat();
    glm::mat4 cam_proj = cam.projectionMat();
    glm::mat4 model = glm::mat4(1.0f);

    GLuint view_loc = glGetUniformLocation(opengl_data.shader_program, "uView");
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(cam_view));

    GLuint proj_loc = glGetUniformLocation(opengl_data.shader_program, "uProjection");
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(cam_proj));

    GLuint model_loc = glGetUniformLocation(opengl_data.shader_program, "uModel");
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(opengl_data.vao);
    glDrawArrays(GL_POINTS, 0, world.n);
}

void gfx::OpenGLData::createVertexObjects() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferStorage(GL_ARRAY_BUFFER,
                    MAX_BODIES * 3 * sizeof(float),
                    nullptr,
                    GL_MAP_WRITE_BIT |
                    GL_MAP_PERSISTENT_BIT |
                    GL_MAP_COHERENT_BIT);

    // XXX: maybe replace 3 with DIM constant
    glBufferData(GL_ARRAY_BUFFER, MAX_BODIES * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);

    vbo_buffer = (float *) glMapBufferRange(GL_ARRAY_BUFFER,
                                            0,
                                            MAX_BODIES * 3 * sizeof(float),
                                            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
}

static std::string readShaderSource(const std::string& filePath) {
    std::ifstream fileStream(filePath);
    
    if (!fileStream.is_open()) {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

void gfx::OpenGLData::loadShadersFromFiles(std::string vertex_shader_path, std::string frag_shader_path) {
    std::string vertex_shader_str = readShaderSource(vertex_shader_path);
    std::string frag_shader_str = readShaderSource(frag_shader_path);

    const char* vertex_shader_src = vertex_shader_str.c_str();
    const char* frag_shader_src   = frag_shader_str.c_str();

    loadShaders(vertex_shader_src, frag_shader_src);
}

void gfx::OpenGLData::loadShaders(const char *vertex_shader_src, const char *frag_shader_src) {
    // vertex shader
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
    glCompileShader(vertex_shader);

    // check vertex shader compiled correctly
    {
        int success;
        char info_log[512];
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << info_log << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // fragment shader
    unsigned int frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &frag_shader_src, NULL);
    glCompileShader(frag_shader);

    // check fragment shader compiled correctly
    {
        int success;
        char info_log[512];
        glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(frag_shader, 512, NULL, info_log);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << info_log << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // create shader program
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);

    // check shader program linked correctly
    {
        int success;
        char info_log[512];
        glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader_program, 512, NULL, info_log);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // use shader program
    glUseProgram(shader_program);

    // delete shaders
    glDeleteShader(vertex_shader);
    glDeleteShader(frag_shader);
}
