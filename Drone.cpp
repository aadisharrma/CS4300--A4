#include "Drone.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// Store a single VAO for our unit cube:
static GLuint gCubeVAO = 0;
static bool   gDroneGeometryInitialized = false;

// Create a unit cube in a VAO for repeated draw calls
void initDroneGeometry()
{
    if (gDroneGeometryInitialized) return;

    float vertices[] = {
        //   X      Y      Z
        // front
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        // back
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        // left
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        // right
         0.5f,  0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
        // top
        -0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        // bottom
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

    glBindVertexArray(0);

    gDroneGeometryInitialized = true;
}

// Helper to draw a cube with a given model transform & shader
static void drawCube(const glm::mat4& model, GLuint shaderProg)
{
    // Make sure we’re using the correct shader
    glUseProgram(shaderProg);

    // Upload the model matrix
    GLint modelLoc = glGetUniformLocation(shaderProg, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    glBindVertexArray(gCubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// Draw the drone with the specified propeller angle & given shader
void drawDrone(float propellerAngle, GLuint shaderProgram)
{
    float radSpin = glm::radians(propellerAngle);

    // Overall transform
    glm::mat4 droneTransform = glm::mat4(1.0f);
    droneTransform = glm::scale(droneTransform, glm::vec3(1.2f));
    droneTransform = glm::translate(droneTransform, glm::vec3(0.0f, 0.2f, 0.0f));

    // Main body
    {
        glm::mat4 body = droneTransform;
        body = glm::scale(body, glm::vec3(1.5f, 0.4f, 0.6f));
        drawCube(body, shaderProgram);
    }

    // Nose
    {
        glm::mat4 nose = droneTransform;
        nose = glm::translate(nose, glm::vec3(0.0f, 0.0f, 0.6f));
        nose = glm::scale(nose, glm::vec3(0.4f, 0.3f, 0.4f));
        drawCube(nose, shaderProgram);
    }

    // Left arm
    {
        glm::mat4 armL = droneTransform;
        armL = glm::translate(armL, glm::vec3(-1.2f, 0.0f, 0.0f));
        armL = glm::scale(armL, glm::vec3(0.3f, 0.15f, 0.8f));
        drawCube(armL, shaderProgram);
    }

    // Right arm
    {
        glm::mat4 armR = droneTransform;
        armR = glm::translate(armR, glm::vec3(1.2f, 0.0f, 0.0f));
        armR = glm::scale(armR, glm::vec3(0.3f, 0.15f, 0.8f));
        drawCube(armR, shaderProgram);
    }

    // Propellers
    auto drawPropeller = [&](float xOffset)
    {
        // hub
        {
            glm::mat4 hub = droneTransform;
            hub = glm::translate(hub, glm::vec3(xOffset, 0.3f, 0.0f));
            hub = glm::rotate(hub, radSpin, glm::vec3(0.0f, 1.0f, 0.0f));
            hub = glm::scale(hub, glm::vec3(0.1f));
            drawCube(hub, shaderProgram);
        }
        // 4 blades
        for(int i = 0; i < 4; i++)
        {
            glm::mat4 blade = droneTransform;
            blade = glm::translate(blade, glm::vec3(xOffset, 0.3f, 0.0f));
            blade = glm::rotate(blade, radSpin, glm::vec3(0.0f, 1.0f, 0.0f));
            blade = glm::rotate(blade, glm::radians(90.0f * i), glm::vec3(0.0f, 1.0f, 0.0f));
            blade = glm::translate(blade, glm::vec3(0.0f, 0.0f, 0.2f));
            blade = glm::scale(blade, glm::vec3(0.05f, 0.02f, 0.4f));
            drawCube(blade, shaderProgram);
        }
    };
    drawPropeller(-1.2f);
    drawPropeller(1.2f);

    // Legs
    {
        glm::mat4 legL = droneTransform;
        legL = glm::translate(legL, glm::vec3(-0.5f, -0.3f, 0.0f));
        legL = glm::scale(legL, glm::vec3(0.1f, 0.4f, 0.1f));
        drawCube(legL, shaderProgram);

        glm::mat4 legR = droneTransform;
        legR = glm::translate(legR, glm::vec3(0.5f, -0.3f, 0.0f));
        legR = glm::scale(legR, glm::vec3(0.1f, 0.4f, 0.1f));
        drawCube(legR, shaderProgram);
    }
}

void cleanupDrone()
{
    if (gCubeVAO)
    {
        glDeleteVertexArrays(1, &gCubeVAO);
        gCubeVAO = 0;
    }
    gDroneGeometryInitialized = false;
}
