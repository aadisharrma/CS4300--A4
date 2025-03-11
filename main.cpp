#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>

#include "Drone.h"
#include "ShaderProgram.h"


// Window size
static int gWindowWidth  = 800;
static int gWindowHeight = 600;

// Part 2: Propeller & Roll
static float gPropAngle   = 0.0f;   // degrees
static float gPropSpeed   = 180.0f; // deg/sec
static bool  gIsRolling   = false;
static float gRollAngle   = 0.0f;   // 0..360
static float gRollSpeed   = 180.0f; // deg/sec

// Part 3: Drone pos/orientation
static glm::vec3 gDronePos = glm::vec3(0.f, 1.f, 0.f);
// We'll start with yaw = -45 so the drone faces the camera 1 vantage
static float gDroneYaw   = -45.f; 
static float gDronePitch = 0.f;

// Movement factor
static float gMoveFactor  = 0.01f;  // distance per (propSpeed * dt)
static float gTurnRate    = 90.0f;  // deg/sec for arrow keys

// Part 4: Multiple cameras
// We'll START with camera 1 so we see the drone from an angle
static int   gCurrentCamera = 1;  
static float gChopperAngle  = 0.0f; 
static float gChopperSpeed  = 30.f; // deg/sec overhead orbit


// GLFW Callbacks
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    gWindowWidth  = width;
    gWindowHeight = height;
    glViewport(0, 0, width, height);
}

// Build a forward vector from yaw/pitch
static glm::vec3 getForwardVector(float yawDeg, float pitchDeg)
{
    glm::vec3 baseForward(0.f, 0.f, 1.f);
    glm::mat4 transform(1.0f);

    // Yaw around Y, then pitch around X
    transform = glm::rotate(transform, glm::radians(yawDeg),   glm::vec3(0,1,0));
    transform = glm::rotate(transform, glm::radians(pitchDeg), glm::vec3(1,0,0));

    glm::vec4 dir4 = transform * glm::vec4(baseForward, 0.0f);
    return glm::normalize(glm::vec3(dir4));
}

// Process user input
static void processInput(GLFWwindow* window, float dt)
{
    // Close with ESC
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Speed up/slow down propellers
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        gPropSpeed += 50.f * dt;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        gPropSpeed -= 50.f * dt;
        if(gPropSpeed < 0.f) gPropSpeed = 0.f;
    }

    // Single 360 roll if not already rolling
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !gIsRolling)
    {
        gIsRolling = true;
        gRollAngle = 0.f;
    }

    // Move forward/back with '='/'-'
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
    {
        float dist = gPropSpeed * gMoveFactor * dt;
        gDronePos += getForwardVector(gDroneYaw, gDronePitch) * dist;
    }
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
    {
        float dist = gPropSpeed * gMoveFactor * dt;
        gDronePos -= getForwardVector(gDroneYaw, gDronePitch) * dist;
    }

    // Turn with arrow keys
    float turn = gTurnRate * dt;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        gDroneYaw   -= turn;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        gDroneYaw   += turn;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        gDronePitch += turn;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        gDronePitch -= turn;

    // Reset with 'r' => different orientation from startup
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        // Put drone at center above ground, face +Z
        gDronePos   = glm::vec3(0.f, 1.f, 0.f);
        gDroneYaw   = 0.f;  
        gDronePitch = 0.f;
        gIsRolling  = false;
        gRollAngle  = 0.f;

        // (Optionally reset gPropSpeed etc. if you like)
        // gPropSpeed = 180.f;
    }

    // Switch cameras
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        gCurrentCamera = 1;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        gCurrentCamera = 2;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        gCurrentCamera = 3;
}


// Return a view matrix for whichever camera is active
static glm::mat4 getViewMatrix(float dt)
{
    // Even if camera 2 not active, we update chopper angle
    gChopperAngle += gChopperSpeed * dt;
    if(gChopperAngle > 360.f)
        gChopperAngle = fmod(gChopperAngle, 360.f);

    switch(gCurrentCamera)
    {
    case 1:
    {
        // Camera 1: an angled vantage from (6,3,6)
        // We'll look at roughly (0,1,0)
        glm::vec3 camPos(6.f, 3.f, 6.f);
        glm::vec3 target(0.f, 1.f, 0.f);
        glm::vec3 up(0.f, 1.f, 0.f);
        return glm::lookAt(camPos, target, up);
    }
    case 2:
    {
        // Camera 2: overhead "chopper" orbit
        float radius = 10.f;
        float x = radius * cos(glm::radians(gChopperAngle));
        float z = radius * sin(glm::radians(gChopperAngle));
        glm::vec3 camPos(x, 8.f, z);
        glm::vec3 target(0.f, 0.f, 0.f);
        glm::vec3 up(0.f, 1.f, 0.f);
        return glm::lookAt(camPos, target, up);
    }
    case 3:
    {
        // Camera 3: first-person from the drone’s nose
        // ignoring roll for less disorienting spin
        glm::vec3 forward = getForwardVector(gDroneYaw, gDronePitch);
        glm::vec3 camPos  = gDronePos + forward * 0.3f;
        glm::vec3 target  = camPos + forward;

        // Build an up vector from yaw & pitch only
        glm::mat4 rot(1.f);
        rot = glm::rotate(rot, glm::radians(gDroneYaw),   glm::vec3(0,1,0));
        rot = glm::rotate(rot, glm::radians(gDronePitch), glm::vec3(1,0,0));
        glm::vec3 up = glm::vec3(rot * glm::vec4(0,1,0,0));

        return glm::lookAt(camPos, target, up);
    }
    default:
    {
        // Fallback vantage (shouldn't happen unless you define camera=0 etc.)
        return glm::lookAt(glm::vec3(0.f,3.f,8.f), glm::vec3(0.f,0.f,0.f), glm::vec3(0.f,1.f,0.f));
    }
    }
}

int main()
{
    // Init GLFW
    if(!glfwInit())
    {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(gWindowWidth, gWindowHeight, "Drone (Parts1-4) with AngledCam+Reset", nullptr, nullptr);
    if(!window)
    {
        std::cerr << "Failed to create GLFW window\n";
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

    // Minimal vertex + fragment shaders
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

    // Build & link our shader program
    GLuint shaderProg = createShaderProgram(vertexSrc, fragmentSrc);

    // Initialize the drone geometry
    initDroneGeometry();

    float lastTime = (float)glfwGetTime();

    // Main render loop
    while(!glfwWindowShouldClose(window))
    {
        float currentTime = (float)glfwGetTime();
        float dt = currentTime - lastTime;
        lastTime = currentTime;

        // Process input
        processInput(window, dt);

        // Propellers
        gPropAngle += gPropSpeed * dt;
        if(gPropAngle > 360.f)
            gPropAngle = fmod(gPropAngle, 360.f);

        // Roll
        if(gIsRolling)
        {
            gRollAngle += gRollSpeed * dt;
            if(gRollAngle >= 360.f)
            {
                gRollAngle = 0.f;
                gIsRolling = false;
            }
        }

        // Clear
        glClearColor(0.12f, 0.12f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader
        glUseProgram(shaderProg);

        // Camera
        glm::mat4 view = getViewMatrix(dt);
        GLint viewLoc = glGetUniformLocation(shaderProg, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Projection
        glm::mat4 projection = glm::perspective(
            glm::radians(45.f),
            (float)gWindowWidth / (float)gWindowHeight,
            0.1f, 100.f
        );
        GLint projLoc = glGetUniformLocation(shaderProg, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Color
        GLint colorLoc = glGetUniformLocation(shaderProg, "objectColor");
        glUniform3f(colorLoc, 1.f, 1.f, 1.f); // white

        // Draw the drone
        drawDrone(
            gPropAngle,
            gRollAngle,
            gDroneYaw,
            gDronePitch,
            gDronePos,
            shaderProg
        );

        // (Optional) draw ground or other objects here

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanupDrone();
    glDeleteProgram(shaderProg);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
