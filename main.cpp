#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>

#include "Drone.h"
#include "ShaderProgram.h"

static int   gWindowWidth  = 800;
static int   gWindowHeight = 600;

// -- Part 2: Propeller & Roll --
static float gPropAngle      = 0.0f;   // current propeller rotation
static float gPropSpeed      = 180.0f; // degrees/second
static bool  gIsRolling      = false;  // is the drone doing a 360 roll?
static float gRollAngle      = 0.0f;   // 0..360
static float gRollSpeed      = 180.0f; // degrees/second for the roll

// -- Part 3: Position & Orientation --
static glm::vec3 gDronePos   = glm::vec3(0.0f, 1.0f, 0.0f); // start above ground
static float gDroneYaw       = 0.0f;  // rotate left/right around Y
static float gDronePitch     = 0.0f;  // rotate up/down around X

// Movement rate per unit prop-speed, tweak as desired
static float gMoveFactor = 0.01f;     // how far to move per (speed * dt)
// Turn rate for arrow keys
static float gTurnRate   = 90.0f;     // degrees per second

// Vertex + Fragment Shaders
static const char* vertexSrc = R"(
#version 330 core
layout(location=0) in vec3 aPos;

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

// GLFW Callbacks
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    gWindowWidth  = width;
    gWindowHeight = height;
    glViewport(0, 0, width, height);
}

// For Part 3, we want to move the drone forward/back in the direction it faces.
// Let's define a function that returns the "forward" direction based on yaw & pitch.
static glm::vec3 getForwardVector()
{
    // If drone faces +Z when yaw=0, pitch=0:
    // We'll rotate baseForward = (0,0,1) by yaw around Y, then pitch around X.
    glm::vec3 baseForward(0.0f, 0.0f, 1.0f);

    glm::mat4 transform(1.0f);
    transform = glm::rotate(transform, glm::radians(gDroneYaw),   glm::vec3(0,1,0));
    transform = glm::rotate(transform, glm::radians(gDronePitch), glm::vec3(1,0,0));

    glm::vec4 dir4 = transform * glm::vec4(baseForward, 0.0f);
    return glm::vec3(dir4);
}

static void processInput(GLFWwindow* window)
{
    // Quit on ESC
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Increase/Decrease propeller speed
    if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        gPropSpeed += 50.0f;
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        gPropSpeed = std::max(0.0f, gPropSpeed - 50.0f);
    }

    // Sideways roll
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !gIsRolling)
    {
        gIsRolling = true;
        gRollAngle = 0.0f;
    }

    // Forward with '=' (i.e. "plus" on most keyboards)
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
    {
        float movePerSec = gPropSpeed * gMoveFactor;
        gDronePos += getForwardVector() * movePerSec;
    }
    // Backward with '-'
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
    {
        float movePerSec = gPropSpeed * gMoveFactor;
        gDronePos -= getForwardVector() * movePerSec;
    }

    // Turn with arrow keys
    float dt = 0.016f;  // or measure real dt; either way works for simple turning
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        gDroneYaw -= gTurnRate * dt;
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        gDroneYaw += gTurnRate * dt;
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        gDronePitch += gTurnRate * dt;
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        gDronePitch -= gTurnRate * dt;

    // Reset with 'r'
    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        gDronePos   = glm::vec3(0.0f, 1.0f, 0.0f);
        gDroneYaw   = 0.0f;
        gDronePitch = 0.0f;
        gIsRolling  = false;
        gRollAngle  = 0.0f;
    }
}


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

    GLFWwindow* window = glfwCreateWindow(gWindowWidth, gWindowHeight, "Drone with Movement", nullptr, nullptr);
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

    // Initialize drone geometry
    initDroneGeometry();

    // We’ll measure real dt per frame for propellers & roll
    float lastTime = (float)glfwGetTime();

    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        // Compute real dt
        float currentTime = (float)glfwGetTime();
        float dt = currentTime - lastTime;
        lastTime = currentTime;

        // PART 2: Animate propellers
        gPropAngle += gPropSpeed * dt;
        if(gPropAngle >= 360.f)
            gPropAngle = fmod(gPropAngle, 360.f);

        // If rolling, update rollAngle
        if(gIsRolling)
        {
            gRollAngle += gRollSpeed * dt;
            if(gRollAngle >= 360.f)
            {
                gRollAngle = 0.0f;
                gIsRolling = false;
            }
        }

        // Clear
        glClearColor(0.12f, 0.12f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(shaderProgram);

        // Camera
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.f, 2.f, 6.f), // camera pos
            glm::vec3(0.f, 0.f, 0.f), // looking at origin
            glm::vec3(0.f, 1.f, 0.f)
        );
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 projection = glm::perspective(
            glm::radians(45.f),
            (float)gWindowWidth / (float)gWindowHeight,
            0.1f, 100.f
        );
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // A simple uniform color
        GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
        glUniform3f(colorLoc, 0.9f, 0.9f, 0.9f);

        // Draw the drone with the new parameters
        drawDrone(
            gPropAngle,
            gRollAngle,
            gDroneYaw,
            gDronePitch,
            gDronePos,
            shaderProgram
        );

        // Swap and poll
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
