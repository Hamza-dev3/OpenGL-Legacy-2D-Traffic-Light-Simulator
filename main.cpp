#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int   lightState = 0;
float activeAlpha = 1.0f;
float xOffset = 0.0f;
float lastSwitchTime = 0.0f;
float lightDuration[3] = { 3.5f, 0.5f, 3.5f };
float carX = -1.2f;

const char* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"uniform float uXOffset;\n"
"uniform float uYOffset;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos.x + uXOffset, aPos.y + uYOffset, 0.0, 1.0);\n"
"}\0";

const char* fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec3  uColor;\n"
"uniform float uAlpha;\n"
"void main()\n"
"{\n"
"    FragColor = vec4(uColor, uAlpha);\n"
"}\0";

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        activeAlpha += 0.01f;
        if (activeAlpha > 1.0f) activeAlpha = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        activeAlpha -= 0.01f;
        if (activeAlpha < 0.1f) activeAlpha = 0.1f;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) xOffset -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) xOffset += 0.01f;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Traffic Light", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cout << "GLEW failed\n"; return -1; }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, nullptr);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fs);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    int locX = glGetUniformLocation(shaderProgram, "uXOffset");
    int locY = glGetUniformLocation(shaderProgram, "uYOffset");
    int locColor = glGetUniformLocation(shaderProgram, "uColor");
    int locAlpha = glGetUniformLocation(shaderProgram, "uAlpha");

    float vertices[] = {
        // --- POLE --- (vertex index 0, 6 vertices)
        -0.03f, -0.85f,   0.03f, -0.85f,   0.03f,  0.10f,
        -0.03f, -0.85f,   0.03f,  0.10f,  -0.03f,  0.10f,

        // --- HOUSING BOX --- (vertex index 6, 6 vertices)
        -0.13f, -0.10f,   0.13f, -0.10f,   0.13f,  0.85f,
        -0.13f, -0.10f,   0.13f,  0.85f,  -0.13f,  0.85f,

        // --- LIGHT SQUARE --- (vertex index 12, 6 vertices) reused 3x
        -0.08f, -0.08f,   0.08f, -0.08f,   0.08f,  0.08f,
        -0.08f, -0.08f,   0.08f,  0.08f,  -0.08f,  0.08f,

        // --- CAR BODY --- (vertex index 18, 6 vertices)
        -0.15f, -0.05f,   0.15f, -0.05f,   0.15f,  0.05f,
        -0.15f, -0.05f,   0.15f,  0.05f,  -0.15f,  0.05f,

        // --- CAR ROOF --- (vertex index 24, 6 vertices)
        -0.08f,  0.05f,   0.08f,  0.05f,   0.08f,  0.12f,
        -0.08f,  0.05f,   0.08f,  0.12f,  -0.08f,  0.12f,

        // --- ROAD --- (vertex index 30, 6 vertices)
        -1.0f, -0.82f,    1.0f, -0.82f,    1.0f, -0.60f,
        -1.0f, -0.82f,    1.0f, -0.60f,   -1.0f, -0.60f,

        // --- ROAD DASHES --- (vertex index 36, 8 vertices = 4 dashes x 2 points)
        -1.0f, -0.71f,   -0.6f, -0.71f,
        -0.4f, -0.71f,    0.0f, -0.71f,
         0.2f, -0.71f,    0.6f, -0.71f,
         0.7f, -0.71f,    1.0f, -0.71f,
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = (float)glfwGetTime();
        if (currentTime - lastSwitchTime >= lightDuration[lightState])
        {
            lightState = (lightState + 1) % 3;
            activeAlpha = 1.0f;
            lastSwitchTime = currentTime;
        }

        processInput(window);

        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // --- Draw ROAD ---
        glUniform1f(locX, 0.0f);
        glUniform1f(locY, 0.0f);
        glUniform3f(locColor, 0.20f, 0.20f, 0.20f);
        glUniform1f(locAlpha, 1.0f);
        glDrawArrays(GL_TRIANGLES, 30, 6);

        // --- Draw ROAD DASHES ---
        glUniform3f(locColor, 0.9f, 0.85f, 0.3f); // locX/locY still 0.0f from above
        glLineWidth(3.0f);
        glDrawArrays(GL_LINES, 36, 8);
        glLineWidth(1.0f);

        // --- Draw POLE ---
        glUniform1f(locX, xOffset);
        glUniform1f(locY, 0.0f);
        glUniform3f(locColor, 0.30f, 0.30f, 0.35f);
        glUniform1f(locAlpha, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // --- Draw HOUSING BOX ---
        glUniform1f(locX, xOffset);
        glUniform1f(locY, 0.0f);
        glUniform3f(locColor, 0.05f, 0.05f, 0.08f);
        glUniform1f(locAlpha, 1.0f);
        glDrawArrays(GL_TRIANGLES, 6, 6);

        // --- Draw RED LIGHT ---
        glUniform1f(locX, xOffset);
        glUniform1f(locY, 0.62f);
        glUniform3f(locColor, 1.0f, 0.15f, 0.15f);
        glUniform1f(locAlpha, (lightState == 0) ? activeAlpha : 0.15f);
        glDrawArrays(GL_TRIANGLES, 12, 6);

        // --- Draw YELLOW LIGHT ---
        glUniform1f(locX, xOffset);
        glUniform1f(locY, 0.37f);
        glUniform3f(locColor, 1.0f, 0.85f, 0.0f);
        glUniform1f(locAlpha, (lightState == 1) ? activeAlpha : 0.15f);
        glDrawArrays(GL_TRIANGLES, 12, 6);

        // --- Draw GREEN LIGHT ---
        glUniform1f(locX, xOffset);
        glUniform1f(locY, 0.12f);
        glUniform3f(locColor, 0.0f, 0.95f, 0.3f);
        glUniform1f(locAlpha, (lightState == 2) ? activeAlpha : 0.15f);
        glDrawArrays(GL_TRIANGLES, 12, 6);

        // --- Update car ---
        float stopLine = xOffset - 0.3f;

        bool atStopLine = (lightState != 2) && (carX >= stopLine) && (carX < stopLine + 0.01f);

        if (!atStopLine)
        {
            carX += 0.005f;
            if (carX > 1.3f) carX = -1.3f;
        }


        // --- Draw CAR BODY ---
        glUniform1f(locX, carX);
        glUniform1f(locY, -0.71f);
        glUniform3f(locColor, 0.15f, 0.35f, 0.9f);
        glUniform1f(locAlpha, 1.0f);
        glDrawArrays(GL_TRIANGLES, 18, 6);

        // --- Draw CAR ROOF ---
        glUniform1f(locX, carX);
        glUniform1f(locY, -0.71f);
        glUniform3f(locColor, 0.1f, 0.25f, 0.75f);
        glUniform1f(locAlpha, 1.0f);
        glDrawArrays(GL_TRIANGLES, 24, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
