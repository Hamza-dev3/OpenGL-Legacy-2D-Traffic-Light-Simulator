#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ==========================================
// Global state — controlled by keyboard
//   lightState  : which light is ON (0=Red, 1=Yellow, 2=Green)
//   activeAlpha : brightness of the active light (W/S keys)
//   xOffset     : horizontal position of the whole scene (A/D keys)
// ==========================================
int   lightState = 0;
float activeAlpha = 1.0f;
float xOffset = 0.0f;
float lastSwitchTime = 0.0f;
float lightDuration[3] = { 10.0f, 2.0f, 20.0f }; // Red=10s, Yellow=2s, Green=10s


// ==========================================
// Vertex Shader
// Receives: 2D position (X, Y) at location 0
// Uniforms:
//   uXOffset : moves the drawn shape left/right
//   uYOffset : moves the drawn shape up/down (used to position each light)
// ==========================================
const char* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"uniform float uXOffset;\n"
"uniform float uYOffset;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos.x + uXOffset, aPos.y + uYOffset, 0.0, 1.0);\n"
"}\0";

// ==========================================
// Fragment Shader
// Uniforms:
//   uColor : RGB color of this shape
//   uAlpha : transparency (1.0 = solid, 0.0 = invisible)
// ==========================================
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

// ==========================================
// Input handling
//   SPACE : advance light (Red -> Yellow -> Green -> Red)
//   W / S : increase / decrease active light brightness
//   A / D : move the whole traffic light left / right
//   ESC   : close the window
// ==========================================
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // W / S : increase / decrease active light brightness
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

    // A / D : move the traffic light left / right
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) xOffset -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) xOffset += 0.01f;
}


int main()
{
    // --- 1. Init GLFW ---
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

    // --- 2. Init GLEW ---
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cout << "GLEW failed\n"; return -1; }

    // --- 3. Enable blending so uAlpha works ---
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- 4. Compile shaders ---
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

    // --- 5. Get uniform locations ---
    int locX = glGetUniformLocation(shaderProgram, "uXOffset");
    int locY = glGetUniformLocation(shaderProgram, "uYOffset");
    int locColor = glGetUniformLocation(shaderProgram, "uColor");
    int locAlpha = glGetUniformLocation(shaderProgram, "uAlpha");

    // ==========================================
    // 6. ALL vertex data in ONE array
    //
    // Every shape is 2 triangles (6 vertices), 2 floats each = 12 floats per shape
    // Shapes in order: pole, box, red light, yellow light, green light
    //
    // We use uYOffset = 0 for pole and box (they don't move vertically).
    // We use uYOffset per light to place each one inside the box.
    //
    // Layout of one rectangle (2 triangles):
    //   tri 1: bottom-left, bottom-right, top-right
    //   tri 2: bottom-left, top-right,    top-left
    // ==========================================
    float vertices[] = {
        // --- POLE (thin tall rectangle) ---
        -0.03f, -0.85f,    0.03f, -0.85f,    0.03f,  0.10f,   // triangle 1
        -0.03f, -0.85f,    0.03f,  0.10f,   -0.03f,  0.10f,   // triangle 2

        // --- HOUSING BOX (wider rectangle) ---
        -0.13f, -0.10f,    0.13f, -0.10f,    0.13f,  0.85f,   // triangle 1
        -0.13f, -0.10f,    0.13f,  0.85f,   -0.13f,  0.85f,   // triangle 2

        // --- LIGHT SQUARE (small square, centered at origin; moved by uYOffset) ---
        -0.08f, -0.08f,    0.08f, -0.08f,    0.08f,  0.08f,   // triangle 1
        -0.08f, -0.08f,    0.08f,  0.08f,   -0.08f,  0.08f    // triangle 2
    };
    // Note: the light square is defined only ONCE.
    // We draw it 3 times with different uYOffset and uColor to make Red/Yellow/Green.

    // --- 7. Upload to GPU (one VAO, one VBO) ---
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Attribute 0: 2 floats per vertex, stride = 2 floats, no offset
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // ==========================================
    // 8. Render loop
    // ==========================================
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

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // dark background
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // --- Draw POLE ---
        // vertices start at index 0, 6 vertices (12 floats / 2 = 6 vertices)
        glUniform1f(locX, xOffset);
        glUniform1f(locY, 0.0f);
        glUniform3f(locColor, 0.45f, 0.45f, 0.45f); // grey
        glUniform1f(locAlpha, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);            // vertices 0–5

        // --- Draw HOUSING BOX ---
        // vertices start at index 6
        glUniform1f(locX, xOffset);
        glUniform1f(locY, 0.0f);
        glUniform3f(locColor, 0.1f, 0.1f, 0.1f);    // near-black
        glUniform1f(locAlpha, 1.0f);
        glDrawArrays(GL_TRIANGLES, 6, 6);            // vertices 6–11

        // --- Draw RED LIGHT ---
        // light square vertices start at index 12
        // uYOffset = 0.62 places it at the top of the box
        // alpha = activeAlpha if this is the active light, else 0.15 (dim)
        glUniform1f(locX, xOffset);
        glUniform1f(locY, 0.62f);
        glUniform3f(locColor, 1.0f, 0.0f, 0.0f);    // red
        glUniform1f(locAlpha, (lightState == 0) ? activeAlpha : 0.15f);
        glDrawArrays(GL_TRIANGLES, 12, 6);           // vertices 12–17

        // --- Draw YELLOW LIGHT ---
        // same shape, moved to middle of box
        glUniform1f(locX, xOffset);
        glUniform1f(locY, 0.37f);
        glUniform3f(locColor, 1.0f, 0.8f, 0.0f);    // yellow
        glUniform1f(locAlpha, (lightState == 1) ? activeAlpha : 0.15f);
        glDrawArrays(GL_TRIANGLES, 12, 6);           // same vertices, different uniforms

        // --- Draw GREEN LIGHT ---
        // same shape, moved to bottom of box
        glUniform1f(locX, xOffset);
        glUniform1f(locY, 0.12f);
        glUniform3f(locColor, 0.0f, 0.9f, 0.0f);    // green
        glUniform1f(locAlpha, (lightState == 2) ? activeAlpha : 0.15f);
        glDrawArrays(GL_TRIANGLES, 12, 6);           // same vertices, different uniforms

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- 9. Cleanup ---
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
