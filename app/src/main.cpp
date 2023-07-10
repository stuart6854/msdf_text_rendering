#include "font.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <format>
#include <iostream>

/*
 * NOTES:
 * - Coord System = TopLeft(0, 0), BottomRight(DisplayWidth, DisplayHeight)
 *
 *
 */

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

const std::string MSDFTextVertexShaderSource = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in vec2 in_texCoord;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec2 out_texCoord;

uniform mat4 u_projMatrix;

void main()
{
    out_color = in_color;
    out_texCoord = in_texCoord;
    gl_Position = u_projMatrix * vec4(in_position, 1.0);
}
)";

const std::string MSDFTextFragmentShaderSource = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 in_color;
layout (location = 1) in vec2 in_texCoord;

layout(location = 0) out vec4 out_fragColor;

uniform sampler2D u_fontAtlas;

const float pxRange = 2; // set to distance fields pixel range

float screenPxRange()
{
    vec2 unitRange = vec2(pxRange) / vec2(textureSize(u_fontAtlas, 0));
    vec2 screenTexSize = vec2(1.0) / fwidth(in_texCoord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    vec3 msd = texture(u_fontAtlas, in_texCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange() * (sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    if(opacity == 0.0)
        discard;

    vec4 bgColor = vec4(0.0);
    out_fragColor = mix(bgColor, in_color, opacity);
}
)";

const auto WindowWidth = 1080.0f;
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
    glfwWindowHint(GLFW_SAMPLES, 4);

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

struct Vertex
{
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 texCoord;
};

GLuint textVAO{};
GLuint textVBO{};
GLuint textProgram{};
std::vector<Vertex> textVertices{};
std::uint32_t textVertexCount{};

GLuint create_buffers()
{
#if 0
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec2 texCoord;
    };

    std::vector<Vertex> vertexData{
        { { 10.0f, 10.0f, 0.0f }, { 0.0f, 1.0f } },              // TL
        { { 10.0f, 512 + 10.0f, 0.0f }, { 0.0f, 0.0f } },        // BL
        { { 512 + 10.0f, 512 + 10.0f, 0.0f }, { 1.0f, 0.0f } },  // BR

        { { 512 + 10.0f, 512 + 10.0f, 0.0f }, { 1.0f, 0.0f } },  // BR
        { { 512 + 10.0f, 10.0f, 0.0f }, { 1.0f, 1.0f } },        // TR
        { { 10.0f, 10.0f, 0.0f }, { 0.0f, 1.0f } },              // TL
    };
#endif

    //    GLuint vao{};
    glGenVertexArrays(1, &textVAO);
    glBindVertexArray(textVAO);

    //    GLuint vbo{};
    glGenBuffers(1, &textVBO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, color)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoord)));
    glEnableVertexAttribArray(2);

    return 0;
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

void draw_string(
    glm::vec2 pos, const std::string& string, const glm::mat4& transform, Font& font, std::uint32_t fontSize, const glm::vec4& color)
{
    const auto& geometry = font.get_geometry();
    const auto& metrics = geometry.getMetrics();

    float x = pos.x;  // Align to be pixel perfect
    float y = pos.y;  // Align to be pixel-perfect

    float fsScale = (1.0f / float(metrics.ascenderY - metrics.descenderY)) * float(fontSize);
    y += float(metrics.ascenderY) * fsScale;

    for (std::size_t i = 0; i < string.size(); ++i)
    {
        char character = string[i];
        const auto* glyph = geometry.getGlyph(character);
        if (!glyph)
        {
            glyph = geometry.getGlyph('?');
        }

        double al, ab, ar, at;
        glyph->getQuadAtlasBounds(al, at, ar, ab);
        glm::vec2 texCoordMin((float(al)), float(at));
        glm::vec2 texCoordMax((float(ar)), float(ab));

        double pl, pb, pr, pt;
        glyph->getQuadPlaneBounds(pl, pt, pr, pb);
        glm::vec2 quadTL((float(pl)), float(-pt));  // TopLeft
        glm::vec2 quadBR((float(pr)), float(-pb));  // BottomRight

        quadTL *= fsScale, quadBR *= fsScale;
        quadTL += glm::vec2(x, y);
        quadBR += glm::vec2(x, y);

        quadTL = glm::floor(quadTL);
        quadBR = glm::floor(quadBR);

        float texelWidth = 1.0f / float(font.get_texture_width());
        float texelHeight = 1.0f / float(font.get_texture_height());
        texCoordMin *= glm::vec2(texelWidth, texelHeight);
        texCoordMax *= glm::vec2(texelWidth, texelHeight);

        // #TODO: Potentially cache some of the above

#define ADD_VERTEX(_pos, _color, _texCoord)                        \
    {                                                              \
        auto& vertex = textVertices.emplace_back();                \
        vertex.position = transform * glm::vec4(_pos, 0.0f, 1.0f); \
        vertex.color = _color;                                     \
        vertex.texCoord = _texCoord;                               \
    }

        ADD_VERTEX(quadTL, color, texCoordMin);
        ADD_VERTEX(glm::vec2(quadTL.x, quadBR.y), color, glm::vec2(texCoordMin.x, texCoordMax.y));
        ADD_VERTEX(quadBR, color, texCoordMax);

        ADD_VERTEX(quadBR, color, texCoordMax);
        ADD_VERTEX(glm::vec2(quadBR.x, quadTL.y), color, glm::vec2(texCoordMax.x, texCoordMin.y));
        ADD_VERTEX(quadTL, color, texCoordMin);

        textVertexCount += 6;

        double advance = 0.0;
        if (i < string.size() - 1)
        {
            advance = glyph->getAdvance();
            char nextCharacter = string[i + 1];
            geometry.getAdvance(advance, character, nextCharacter);
        }

        float kerningOffset = 0.0f;
        x += fsScale * float(advance) + kerningOffset;
    }

    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * textVertices.size(), textVertices.data(), GL_DYNAMIC_DRAW);
}

void render(GLuint program, GLuint vao, GLuint texture)
{
    glm::mat4 projMatrix = glm::ortho(0.0f, WindowWidth, WindowHeight, 0.0f);

    glUseProgram(textProgram);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(textVAO);
    glUniformMatrix4fv(glGetUniformLocation(program, "u_projMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));
    glDrawArrays(GL_TRIANGLES, 0, textVertexCount);
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

    textProgram = create_shader_program(MSDFTextVertexShaderSource, MSDFTextFragmentShaderSource);

    Font font("fonts/OpenSans-Regular.ttf");
    //    Font font2("fonts/segoesc.ttf");

    auto texture = create_texture_2d(font.get_texture_width(), font.get_texture_height(), font.get_texture_data(), GL_RGB, false);
    font.set_texture_id(&texture);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //        draw_string({ 0.0f, 0.0f }, "Stuart", glm::mat4(1.0f), font, 24, glm::vec4(1.0f));
        draw_string({ 0.0f, 0.0f }, "abcdefghijklmnopqrtsuvwxyz", glm::mat4(1.0f), font, 24, glm::vec4(1.0f));
        draw_string({ 00.0f, 20.0f }, "Testing 123 if text performs sufficiently?.", glm::mat4(1.0f), font, 60, glm::vec4(1.0f));
        render(textProgram, vao, texture);

        textVertices.clear();
        textVertexCount = 0;

        glfwSwapBuffers(window);
    }

    cleanup(window);

    return 0;
}