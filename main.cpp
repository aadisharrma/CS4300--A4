#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "Drone.h"
#include "ShaderProgram.h"

static int   gWindowWidth  = 800;
static int   gWindowHeight = 600;
static float gPropAngle    = 0.0f;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    gWindowWidth  = width;
    gWindowHeight = height;
    glViewport(0, 0, width, height);
}

static void processInput(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Simple vertex/fragment shaders
static const char* vertexSrc = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

static const char* fragmentSrc = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 objectColor;
void main()
{
    FragColor = vec4(objectColor, 1.0);
}
)";

int main()
{
    if(!glfwInit())
    {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(gWindowWidth, gWindowHeight, "Centered Drone", nullptr, nullptr);
    if(!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to init GLAD\n";
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glEnable(GL_DEPTH_TEST);

    // Build our shader program
    GLuint shaderProgram = createShaderProgram(vertexSrc, fragmentSrc);

    // Initialize the drone geometry (create a cube VAO)
    initDroneGeometry();

    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        // Update the propeller angle
        float dt = (float)glfwGetTime();
        glfwSetTime(0.0);
        gPropAngle += 180.0f * dt; // rotate half a revolution per second

        // Clear
        glClearColor(0.12f, 0.12f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(shaderProgram);

        // Setup camera
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 1.0f, 5.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Projection
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            (float)gWindowWidth / (float)gWindowHeight,
            0.1f, 100.0f
        );
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // objectColor
        GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
        glUniform3f(colorLoc, 0.9f, 0.9f, 0.9f);

        // Draw the drone, passing in the prop angle & current shader
        drawDrone(gPropAngle, shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    cleanupDrone();
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
