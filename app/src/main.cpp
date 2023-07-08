#include "font.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <format>
#include <iostream>

const std::string VertexShaderSource = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 in_Pos;
layout (location = 1) in vec3 in_Color;

layout (location = 0) out vec3 out_Color;

void main() {
    gl_Position = vec4(in_Pos, 1.0);
    out_Color = in_Color;
}
)";

const std::string FragmentShaderSource = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_Color;

layout(location = 0) out vec3 out_fragColor;

void main() {
    out_fragColor = in_Color;
}
)";

template <typename... Args>
void println(std::string_view fmt, Args&&... args)
{
    std::cout << std::vformat(fmt, std::make_format_args(args...)) << std::endl;
}

template <typename... Args>
void errprintln(std::string_view fmt, Args&&... args)
{
    std::cerr << std::vformat(fmt, std::make_format_args(args...)) << std::endl;
}

static void errorCallback(int error, const char* description)
{
    errprintln(description);
}

GLFWwindow* initialise(std::int32_t width, std::int32_t height, std::string_view title)
{
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialise GLFW.");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto* window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    if (!window)
    {
        errprintln("Failed to create GLFW window.");
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGL())
    {
        errprintln("Failed to to initialise GLAD.");
        throw std::runtime_error("Failed to to initialise GLAD.");
        glfwTerminate();
        return nullptr;
    }

    std::string_view renderer = reinterpret_cast<const char* const>(glGetString(GL_RENDERER));
    std::string_view version = reinterpret_cast<const char* const>(glGetString(GL_VERSION));
    println("Renderer: {}", renderer);
    println("OpenGL version: {}", version);

    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    return window;
}

GLuint create_buffers()
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
    };

    std::vector<Vertex> vertexData{
        { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    };

    GLuint vao{};
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo{};
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertexData.size(), vertexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, pos)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, color)));
    glEnableVertexAttribArray(1);

    return vao;
}

GLuint compile_shader(const char* shaderSource, GLenum shaderType)
{
    GLuint shaderHandle = glCreateShader(shaderType);

    glShaderSource(shaderHandle, 1, &shaderSource, nullptr);
    glCompileShader(shaderHandle);

    int success;
    char infoLog[512];
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderHandle, 512, nullptr, infoLog);
        errprintln("Error while compiling shader: {}", infoLog);
    }

    return shaderHandle;
}

GLuint create_shader_program(const std::string& vertexSource, const std::string& fragmentSource)
{
    auto vertexShader = compile_shader(vertexSource.c_str(), GL_VERTEX_SHADER);
    auto fragmentShader = compile_shader(fragmentSource.c_str(), GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetShaderiv(shaderProgram, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
        errprintln("Error while linking shaders: {}", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void render(GLuint program, GLuint vao)
{
    glUseProgram(program);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void cleanup(GLFWwindow* window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main(int argc, char** argv)
{
    std::cout << "MSDF Text Rendering\n";

    Font font("fonts/OpenSans-Regular.ttf");
    //    Font font2("fonts/segoesc.ttf");

    auto* window = initialise(1280, 720, "MSDF Text Rendering");
    auto program = create_shader_program(VertexShaderSource, FragmentShaderSource);
    auto vao = create_buffers();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render(program, vao);

        glfwSwapBuffers(window);
    }

    cleanup(window);

    return 0;
}