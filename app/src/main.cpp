#include "font.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <format>
#include <iostream>

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

GLFWwindow* Initialise(std::int32_t width, std::int32_t height, std::string_view title)
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

void render() {}

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

    auto* window = Initialise(1280, 720, "MSDF Text Rendering");

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render();

        glfwSwapBuffers(window);
    }

    cleanup(window);

    return 0;
}