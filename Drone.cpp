#include "Drone.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// A single VAO for our unit cube geometry:
static GLuint gCubeVAO = 0;
static bool   gDroneGeometryInitialized = false;

/**
 * Create a unit cube ([-0.5,0.5]) in a VAO for repeated draw calls.
 * (You can expand to include normals or texture coords if needed.)
 */
void initDroneGeometry()
{
    if (gDroneGeometryInitialized) return; // do it only once

    float vertices[] = {
        //    X      Y      Z
        // front face
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        // back face
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        // left face
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

        // right face
         0.5f,  0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,

        // top face
        -0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,

        // bottom face
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f
    };

    GLuint VBO;
    glGenVertexArrays(1, &gCubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(gCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind
    glBindVertexArray(0);

    gDroneGeometryInitialized = true;
}

/**
 * Helper to draw our unit cube with the given model transform & shader.
 */
static void drawCube(const glm::mat4& model, GLuint shaderProg)
{
    glUseProgram(shaderProg);
    GLint modelLoc = glGetUniformLocation(shaderProg, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(gCubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

/**
 * Draw the drone. 
 *  - propAngle:  spin angle (in degrees) for the propellers.
 *  - rollAngle:  roll angle (in degrees) about the drone’s front/back axis.
 *  - shaderProg: which shader to use for rendering.
 */
void drawDrone(float propAngle, float rollAngle, GLuint shaderProg)
{
    // Convert degrees to radians for rotations
    float propRad = glm::radians(propAngle);
    float rollRad = glm::radians(rollAngle);

    // The main transform for the drone body
    glm::mat4 droneTransform(1.0f);

    // 1) Apply a roll around the Z-axis (assuming +Z is the "front-back" axis).
    droneTransform = glm::rotate(droneTransform, rollRad, glm::vec3(0.0f, 0.0f, 1.0f));

    // 2) Scale and shift upward so it’s nicely centered
    droneTransform = glm::scale(droneTransform, glm::vec3(1.2f));
    droneTransform = glm::translate(droneTransform, glm::vec3(0.0f, 0.2f, 0.0f));

    // --- MAIN FUSELAGE ---
    {
        glm::mat4 body = droneTransform;
        body = glm::scale(body, glm::vec3(1.5f, 0.4f, 0.6f));
        drawCube(body, shaderProg);
    }

    // --- NOSE (front extension) ---
    {
        glm::mat4 nose = droneTransform;
        nose = glm::translate(nose, glm::vec3(0.0f, 0.0f, 0.6f));
        nose = glm::scale(nose, glm::vec3(0.4f, 0.3f, 0.4f));
        drawCube(nose, shaderProg);
    }

    // --- ARMS (left & right) ---
    {
        glm::mat4 armL = droneTransform;
        armL = glm::translate(armL, glm::vec3(-1.2f, 0.0f, 0.0f));
        armL = glm::scale(armL, glm::vec3(0.3f, 0.15f, 0.8f));
        drawCube(armL, shaderProg);
    }
    {
        glm::mat4 armR = droneTransform;
        armR = glm::translate(armR, glm::vec3(1.2f, 0.0f, 0.0f));
        armR = glm::scale(armR, glm::vec3(0.3f, 0.15f, 0.8f));
        drawCube(armR, shaderProg);
    }

    // --- PROPELLERS ---
    // Spin them around the Y-axis via propRad
    auto drawPropeller = [&](float xOffset)
    {
        // Hub
        {
            glm::mat4 hub = droneTransform;
            hub = glm::translate(hub, glm::vec3(xOffset, 0.3f, 0.0f));
            hub = glm::rotate(hub, propRad, glm::vec3(0.0f, 1.0f, 0.0f));
            hub = glm::scale(hub, glm::vec3(0.1f));
            drawCube(hub, shaderProg);
        }
        // 4 blades
        for(int i = 0; i < 4; i++)
        {
            glm::mat4 blade = droneTransform;
            blade = glm::translate(blade, glm::vec3(xOffset, 0.3f, 0.0f));
            blade = glm::rotate(blade, propRad, glm::vec3(0.0f, 1.0f, 0.0f));
            blade = glm::rotate(blade, glm::radians(90.0f * i), glm::vec3(0.0f, 1.0f, 0.0f));
            blade = glm::translate(blade, glm::vec3(0.0f, 0.0f, 0.2f));
            blade = glm::scale(blade, glm::vec3(0.05f, 0.02f, 0.4f));
            drawCube(blade, shaderProg);
        }
    };
    drawPropeller(-1.2f);
    drawPropeller(1.2f);

    // --- LEGS ---
    {
        glm::mat4 legL = droneTransform;
        legL = glm::translate(legL, glm::vec3(-0.5f, -0.3f, 0.0f));
        legL = glm::scale(legL, glm::vec3(0.1f, 0.4f, 0.1f));
        drawCube(legL, shaderProg);

        glm::mat4 legR = droneTransform;
        legR = glm::translate(legR, glm::vec3(0.5f, -0.3f, 0.0f));
        legR = glm::scale(legR, glm::vec3(0.1f, 0.4f, 0.1f));
        drawCube(legR, shaderProg);
    }
}

void cleanupDrone()
{
    if(gCubeVAO)
    {
        glDeleteVertexArrays(1, &gCubeVAO);
        gCubeVAO = 0;
    }
    gDroneGeometryInitialized = false;
}
