#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>

// Our headers
#include "Drone.h"
#include "ShaderProgram.h"

// Window size
static int gWindowWidth  = 800;
static int gWindowHeight = 600;

// Animation state
static float gPropAngle      = 0.0f;   // current propeller angle
static float gPropSpeed      = 180.0f; // degrees/second
static bool  gIsRolling      = false;  // is the drone rolling right now?
static float gRollAngle      = 0.0f;   // 0..360
static float gRollSpeed      = 180.0f; // degrees/second for the roll

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

// Resize callback
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    gWindowWidth  = width;
    gWindowHeight = height;
    glViewport(0, 0, width, height);
}

static void processInput(GLFWwindow* window)
{
    // Close on Esc
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Press 'f' => speed up propellers
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        gPropSpeed += 50.0f;
    }

    // Press 'd' => slow down propellers
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        gPropSpeed -= 50.0f;
        if(gPropSpeed < 0.0f) gPropSpeed = 0.0f;
    }

    // Press 's' => trigger a sideways roll if not already rolling
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !gIsRolling)
    {
        gIsRolling = true;
        gRollAngle = 0.0f;
    }
}

int main()
{
    if(!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(gWindowWidth, gWindowHeight, "Centered Drone", nullptr, nullptr);
    if(!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Build our shader program
    GLuint shaderProgram = createShaderProgram(vertexSrc, fragmentSrc);

    // Set up the drone geometry (cube VAO)
    initDroneGeometry();

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window))
    {
        // Handle key presses
        processInput(window);

        // Measure delta time to animate smoothly
        float dt = (float)glfwGetTime();
        glfwSetTime(0.0);

        // 1) Update propeller angle
        gPropAngle += gPropSpeed * dt;
        // keep within [0..360)
        if(gPropAngle > 360.0f)
            gPropAngle = fmod(gPropAngle, 360.0f);

        // 2) Update roll if active
        if(gIsRolling)
        {
            gRollAngle += gRollSpeed * dt;
            if(gRollAngle >= 360.0f)
            {
                gRollAngle = 0.0f;
                gIsRolling = false;
            }
        }

        // Clear screen
        glClearColor(0.12f, 0.12f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(shaderProgram);

        // Set up camera / view
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 1.0f, 5.0f), // camera position
            glm::vec3(0.0f, 0.0f, 0.0f), // look at origin
            glm::vec3(0.0f, 1.0f, 0.0f)  // up vector
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

        // Give the drone a color (white/gray here)
        GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
        glUniform3f(colorLoc, 0.9f, 0.9f, 0.9f);

        // Draw the drone, passing in current angles
        drawDrone(gPropAngle, gRollAngle, shaderProgram);

        // Swap buffers
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
