#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>

#include "DroneModel.h"
#include "DroneView.h"
#include "DroneController.h"
#include "ShaderProgram.h"

// Window size
static int gWindowWidth  = 800;
static int gWindowHeight = 600;

// Cameras
static int   gCurrentCamera = 0;  
static float gChopperAngle  = 0.0f; 
static float gChopperSpeed  = 30.f; // deg/sec overhead orbit

//---------------------------------------------
// GLFW Callbacks
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    gWindowWidth  = width;
    gWindowHeight = height;
    glViewport(0, 0, width, height);
}

//---------------------------------------------
// Build a forward vector from yaw/pitch for camera usage
static glm::vec3 getForwardVector(float yawDeg, float pitchDeg)
{
    // If yaw=0 => facing +Z in your local space
    glm::vec3 baseForward(0.f, 0.f, 1.f);
    glm::mat4 transform(1.0f);

    transform = glm::rotate(transform, glm::radians(yawDeg),   glm::vec3(0,1,0));
    transform = glm::rotate(transform, glm::radians(pitchDeg), glm::vec3(1,0,0));

    glm::vec4 dir4 = transform * glm::vec4(baseForward, 0.0f);
    return glm::normalize(glm::vec3(dir4));
}

//---------------------------------------------
// Return a view matrix for whichever camera is active
static glm::mat4 getViewMatrix(float dt, DroneModel& droneModel)
{
    // Keep updating chopper angle for the overhead orbit camera
    gChopperAngle += gChopperSpeed * dt;
    if(gChopperAngle > 360.f)
        gChopperAngle = fmod(gChopperAngle, 360.f);

    switch(gCurrentCamera)
    {
    // CASE 0 = angled vantage at startup
    case 0:
    {
        glm::vec3 camPos(6.f, 3.f, 6.f);
        glm::vec3 target(0.f, 1.f, 0.f);
        glm::vec3 up(0.f, 1.f, 0.f);
        return glm::lookAt(camPos, target, up);
    }
    // CASE 1 = strict top-down
    case 1:
    {
        glm::vec3 camPos(0.f, 15.f, 0.f);  
        glm::vec3 target(0.f, 0.f, 0.f);   
        glm::vec3 up(0.f, 0.f, -1.f);      
        return glm::lookAt(camPos, target, up);
    }
    // CASE 2 = overhead "chopper" orbit
    case 2:
    {
        float radius = 10.f;
        float x = radius * cos(glm::radians(gChopperAngle));
        float z = radius * sin(glm::radians(gChopperAngle));
        glm::vec3 camPos(x, 8.f, z);
        glm::vec3 target(0.f, 0.f, 0.f);
        glm::vec3 up(0.f, 1.f, 0.f);
        return glm::lookAt(camPos, target, up);
    }
    // CASE 3 = first-person from the drone’s nose
    case 3:
    {
        float yaw   = droneModel.getYaw();
        float pitch = droneModel.getPitch();

        glm::vec3 forward = getForwardVector(yaw, pitch);

        // 1) Move the camera forward +1.2f
        // 2) Shift it down ~0.3f
        glm::vec3 camPos = droneModel.getPosition()
                         + forward * 1.2f
                         + glm::vec3(0.f, -0.3f, 0.f);

        glm::vec3 target = camPos + forward;

        // Compute 'up' from yaw/pitch
        glm::mat4 rot(1.f);
        rot = glm::rotate(rot, glm::radians(yaw),   glm::vec3(0,1,0));
        rot = glm::rotate(rot, glm::radians(pitch), glm::vec3(1,0,0));
        glm::vec3 up = glm::vec3(rot * glm::vec4(0,1,0,0));

        return glm::lookAt(camPos, target, up);
    }
    default:
    {
        glm::vec3 camPos(6.f, 3.f, 6.f);
        glm::vec3 target(0.f, 1.f, 0.f);
        glm::vec3 up(0.f, 1.f, 0.f);
        return glm::lookAt(camPos, target, up);
    }
    }
}

//---------------------------------------------
// Process user input (movement, camera switching, etc.)
static void processInput(GLFWwindow* window, float dt,
                         DroneModel& droneModel,
                         DroneController& droneController)
{
    // Close with ESC
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Speed up/slow down propellers with 'f' and 's' keys
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        droneController.increasePropSpeed(50.f * dt);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        droneController.decreasePropSpeed(50.f * dt);

    // Single 360 roll if not already rolling - using 'j' key
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS && !droneController.isRolling())
    {
        droneController.startRoll();
    }

    // Move forward/back with '='/'-'
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
    {
        float dist = droneController.getPropSpeed() * 0.01f * dt;
        droneController.moveForward(dist);
    }
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
    {
        float dist = droneController.getPropSpeed() * 0.01f * dt;
        droneController.moveBackward(dist);
    }

    // Turn with arrow keys
    float turn = 90.0f * dt; // was gTurnRate = 90 deg/sec
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        droneController.turnYaw(-turn);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        droneController.turnYaw(+turn);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        droneController.turnPitch(+turn);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        droneController.turnPitch(-turn);

    // Reset with 'R'
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        droneController.reset();
        gCurrentCamera = 0;
    }

    // Switch cameras: 0 => angled vantage, 1 => top-down, 2 => orbit, 3 => FP
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        gCurrentCamera = 1;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        gCurrentCamera = 2;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        gCurrentCamera = 3;
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
        gCurrentCamera = 0;
}

//---------------------------------------------
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

    GLFWwindow* window = glfwCreateWindow(gWindowWidth, gWindowHeight,
        "Drone (MVC)", nullptr, nullptr);
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

    // Create Model, View, Controller
    DroneModel droneModel;               // holds drone state
    DroneView  droneView;                // handles geometry & rendering
    DroneController droneController(droneModel); // manipulates the model

    // Initialize geometry once
    droneView.initDroneGeometry();

    float lastTime = (float)glfwGetTime();

    // Main render loop
    while(!glfwWindowShouldClose(window))
    {
        float currentTime = (float)glfwGetTime();
        float dt = currentTime - lastTime;
        lastTime = currentTime;

        // Process input
        processInput(window, dt, droneModel, droneController);

        // Update propeller angle
        droneController.updatePropAngle(dt);

        // Update roll
        droneController.updateRoll(dt);

        // Clear
        glClearColor(0.12f, 0.12f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader
        glUseProgram(shaderProg);

        // Camera
        glm::mat4 view = getViewMatrix(dt, droneModel);
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

        // Draw the drone
        droneView.drawDrone(droneModel, shaderProg);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    droneView.cleanupDrone();
    glDeleteProgram(shaderProg);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
