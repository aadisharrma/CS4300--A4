#include "Drone.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

static GLuint gCubeVAO = 0;
static bool   gDroneGeometryInitialized = false;

void initDroneGeometry()
{
    if (gDroneGeometryInitialized) return;

    float vertices[] = {
        //    X,     Y,     Z
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

// A tiny helper to draw the unit cube with a given model transform & current shader
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
 * Draw the drone, coloring its body white but making the nose gray.
 *
 *  - propAngle:  spinning angle for the propellers
 *  - rollAngle:  side roll from Part 2
 *  - yaw, pitch: orientation from Part 3
 *  - position:   world-space position
 *  - shaderProg: the shader to use (must have "objectColor" uniform)
 */
void drawDrone(
    float propAngle,
    float rollAngle,
    float yaw,
    float pitch,
    const glm::vec3& position,
    GLuint shaderProg
)
{
    // Convert degrees to radians
    float propRad  = glm::radians(propAngle);
    float rollRad  = glm::radians(rollAngle);
    float yawRad   = glm::radians(yaw);
    float pitchRad = glm::radians(pitch);

    // Build the overall transform
    glm::mat4 droneTransform(1.0f);

    // 1) Position in world
    droneTransform = glm::translate(droneTransform, position);

    // 2) Yaw around Y, pitch around X
    droneTransform = glm::rotate(droneTransform, yawRad,   glm::vec3(0,1,0));
    droneTransform = glm::rotate(droneTransform, pitchRad, glm::vec3(1,0,0));

    // 3) The sideways roll from Part 2 (Z axis)
    droneTransform = glm::rotate(droneTransform, rollRad, glm::vec3(0,0,1));

    // 4) Scale smaller (e.g. 0.8f if you want it even smaller),
    //    and shift slightly up so it doesn't sink under the ground.
    droneTransform = glm::scale(droneTransform, glm::vec3(1.2f));
    droneTransform = glm::translate(droneTransform, glm::vec3(0.0f, 0.2f, 0.0f));

    // We'll set the color uniform per sub-part
    glUseProgram(shaderProg);
    GLuint colorLoc = glGetUniformLocation(shaderProg, "objectColor");

    // --- MAIN BODY (white) ---
    {
        glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f); // white
        glm::mat4 body = glm::scale(droneTransform, glm::vec3(1.5f, 0.4f, 0.6f));
        drawCube(body, shaderProg);
    }

    // --- NOSE (gray) ---
    {
        glUniform3f(colorLoc, 0.5f, 0.5f, 0.5f); // grey
        glm::mat4 nose = glm::translate(droneTransform, glm::vec3(0.0f, 0.0f, 0.6f));
        nose = glm::scale(nose, glm::vec3(0.4f, 0.3f, 0.4f));
        drawCube(nose, shaderProg);
    }

    // --- ARMS (white again) ---
    {
        glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f); // white
        glm::mat4 armL = glm::translate(droneTransform, glm::vec3(-1.2f, 0.0f, 0.0f));
        armL = glm::scale(armL, glm::vec3(0.3f, 0.15f, 0.8f));
        drawCube(armL, shaderProg);

        glm::mat4 armR = glm::translate(droneTransform, glm::vec3(1.2f, 0.0f, 0.0f));
        armR = glm::scale(armR, glm::vec3(0.3f, 0.15f, 0.8f));
        drawCube(armR, shaderProg);
    }

    // --- PROPELLERS (white) ---
    auto drawPropeller = [&](float xOffset)
    {
        // The spinning is about Y
        glm::mat4 hub = glm::translate(droneTransform, glm::vec3(xOffset, 0.3f, 0.0f));
        hub = glm::rotate(hub, propRad, glm::vec3(0,1,0));
        hub = glm::scale(hub, glm::vec3(0.1f));
        drawCube(hub, shaderProg);

        // 4 blades
        for(int i = 0; i < 4; i++)
        {
            glm::mat4 blade = glm::translate(droneTransform, glm::vec3(xOffset, 0.3f, 0.0f));
            blade = glm::rotate(blade, propRad, glm::vec3(0,1,0));
            blade = glm::rotate(blade, glm::radians(90.0f * i), glm::vec3(0,1,0));
            blade = glm::translate(blade, glm::vec3(0.0f, 0.0f, 0.2f));
            blade = glm::scale(blade, glm::vec3(0.05f, 0.02f, 0.4f));
            drawCube(blade, shaderProg);
        }
    };
    // Keep them white:
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
    drawPropeller(-1.2f);
    drawPropeller(1.2f);

    // --- LEGS (white) ---
    {
        glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
        glm::mat4 legL = glm::translate(droneTransform, glm::vec3(-0.5f, -0.3f, 0.0f));
        legL = glm::scale(legL, glm::vec3(0.1f, 0.4f, 0.1f));
        drawCube(legL, shaderProg);

        glm::mat4 legR = glm::translate(droneTransform, glm::vec3(0.5f, -0.3f, 0.0f));
        legR = glm::scale(legR, glm::vec3(0.1f, 0.4f, 0.1f));
        drawCube(legR, shaderProg);
    }
}

void cleanupDrone()
{
    if(gCubeVAO != 0)
    {
        glDeleteVertexArrays(1, &gCubeVAO);
        gCubeVAO = 0;
    }
    gDroneGeometryInitialized = false;
}
