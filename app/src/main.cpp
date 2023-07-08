#include "font.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <format>
#include <iostream>

const std::string VertexShaderSource = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_texCoord;

layout (location = 0) out vec2 out_texCoord;

uniform mat4 u_projMatrix;

void main()
{
    gl_Position = u_projMatrix * vec4(in_position, 1.0);
    out_texCoord = in_texCoord;
}
)";

const std::string FragmentShaderSource = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 in_texCoord;

layout(location = 0) out vec3 out_fragColor;

uniform sampler2D u_texture;

void main()
{
    out_fragColor = texture(u_texture, in_texCoord).rgb;
}
)";

const auto WindowWidth = 1280.0f;
const auto WindowHeight = 720.0f;

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
        glm::vec2 texCoord;
    };

    std::vector<Vertex> vertexData{
        { { 10.0f, 10.0f, 0.0f }, { 0.0f, 1.0f } },              // TL
        { { 10.0f, 512 + 10.0f, 0.0f }, { 0.0f, 0.0f } },             // BL
        { { 512 + 10.0f, 512 + 10.0f, 0.0f }, { 1.0f, 0.0f } },  // BR

        { { 512 + 10.0f, 512 + 10.0f, 0.0f }, { 1.0f, 0.0f } },  // BR
        { { 512 + 10.0f, 10.0f, 0.0f }, { 1.0f, 1.0f } },        // TR
        { { 10.0f, 10.0f, 0.0f }, { 0.0f, 1.0f } },              // TL
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
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoord)));
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

GLuint create_texture_2d(std::uint32_t width, std::uint32_t height, const void* data, GLenum format, bool generateMipMaps)
{
    GLuint texture{};
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    const int mipMapLevel = 0;
    const GLenum sourceFormat = format;
    const GLenum sourceDataType = GL_UNSIGNED_BYTE;
    glTexImage2D(GL_TEXTURE_2D, mipMapLevel, sourceFormat, width, height, 0, sourceFormat, sourceDataType, data);
    if (generateMipMaps)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    return texture;
}

void render(GLuint program, GLuint vao, GLuint texture)
{
    glm::mat4 projMatrix = glm::ortho(0.0f, WindowWidth, WindowHeight, 0.0f);

    glUseProgram(program);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(vao);
    glUniformMatrix4fv(glGetUniformLocation(program, "u_projMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void cleanup(GLFWwindow* window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main(int argc, char** argv)
{
    std::cout << "MSDF Text Rendering\n";

    auto* window = initialise(WindowWidth, WindowHeight, "MSDF Text Rendering");
    auto program = create_shader_program(VertexShaderSource, FragmentShaderSource);
    auto vao = create_buffers();

    Font font("fonts/OpenSans-Regular.ttf");
    //    Font font2("fonts/segoesc.ttf");

    auto texture = create_texture_2d(font.get_texture_width(), font.get_texture_height(), font.get_texture_data(), GL_RGB, false);
    font.set_texture_id(&texture);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render(program, vao, texture);

        glfwSwapBuffers(window);
    }

    cleanup(window);

    return 0;
}