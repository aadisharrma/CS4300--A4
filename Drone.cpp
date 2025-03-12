#include "Drone.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <iostream>

// Existing globals for the cube
static GLuint gCubeVAO = 0;
static bool   gDroneGeometryInitialized = false;

// NEW globals for the sphere
static GLuint gSphereVAO = 0;
static GLuint gSphereVBO = 0;
static int    gSphereNumVerts = 0;  // how many vertices in our sphere

// ----------------------------------------------------------
// Generate a sphere (positions only) and store in gSphereVAO
static void initSphereGeometry()
{
    if (gSphereVAO != 0) return; // already inited

    const int stacks = 12;
    const int slices = 12;
    const float PI   = 3.14159265359f;

    std::vector<float> verts; 
    verts.reserve(stacks * slices * 6); 

    // We’ll build triangle strips for each stack.
    // Each “stack” is a ring of slices around the sphere.
    for(int i = 0; i < stacks; i++)
    {
        // phi0 and phi1 are the latitudes in radians
        float phi0 = PI * (-0.5f + (float)i / stacks);     
        float phi1 = PI * (-0.5f + (float)(i+1) / stacks); 

        float y0 = std::sin(phi0); 
        float r0 = std::cos(phi0); 
        float y1 = std::sin(phi1);
        float r1 = std::cos(phi1);

        // For each stack, we connect slices in a “strip”
        for(int j = 0; j <= slices; j++)
        {
            float theta = 2.0f * PI * ((float)j / slices);
            float x = std::cos(theta);
            float z = std::sin(theta);

            // (x*r0, y0, z*r0)
            verts.push_back(x * r0);
            verts.push_back(y0);
            verts.push_back(z * r0);

            // (x*r1, y1, z*r1)
            verts.push_back(x * r1);
            verts.push_back(y1);
            verts.push_back(z * r1);
        }
    }

    gSphereNumVerts = (int)verts.size() / 3; // each vertex has 3 floats

    // Create VAO/VBO
    glGenVertexArrays(1, &gSphereVAO);
    glGenBuffers(1, &gSphereVBO);

    glBindVertexArray(gSphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gSphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verts.size(), verts.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

// ----------------------------------------------------------
// Existing initDroneGeometry for the cube
void initDroneGeometry()
{
    if (gDroneGeometryInitialized) return;

    // 1) Initialize the cube VAO (like before)
    float vertices[] = {
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

    // 2) Initialize the sphere for the circular nose
    initSphereGeometry();

    gDroneGeometryInitialized = true;
}

// ----------------------------------------------------------
// Helpers to draw the cube or sphere
static void drawCube(const glm::mat4& model, GLuint shaderProg)
{
    GLint modelLoc = glGetUniformLocation(shaderProg, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(gCubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

static void drawSphere(const glm::mat4& model, GLuint shaderProg)
{
    GLint modelLoc = glGetUniformLocation(shaderProg, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(gSphereVAO);
    // We built triangle strips, but we can just draw them as a big triangle array
    // The sphere geometry is a "strip," so to ensure it works, we can do:
    // glDrawArrays(GL_TRIANGLE_STRIP, 0, gSphereNumVerts); 
    // But we appended multiple strips in one array, so let's do a simple approach:
    glDrawArrays(GL_TRIANGLE_STRIP, 0, gSphereNumVerts);
    glBindVertexArray(0);
}
// ----------------------------------------------------------
void drawDrone(
    float propAngle,
    float rollAngle,
    float yaw,
    float pitch,
    const glm::vec3& position,
    GLuint shaderProg
)
{
    float propRad  = glm::radians(propAngle);
    float rollRad  = glm::radians(rollAngle);
    float yawRad   = glm::radians(yaw);
    float pitchRad = glm::radians(pitch);

    // Base transform
    glm::mat4 drone(1.0f);
    drone = glm::translate(drone, position);
    drone = glm::rotate(drone, yawRad,   glm::vec3(0,1,0));
    drone = glm::rotate(drone, pitchRad, glm::vec3(1,0,0));
    drone = glm::rotate(drone, rollRad,  glm::vec3(0,0,1));

    // Overall scale + slight upward shift
    drone = glm::scale(drone, glm::vec3(1.0f));
    drone = glm::translate(drone, glm::vec3(0.0f, 0.2f, 0.0f));

    glUseProgram(shaderProg);
    GLint colorLoc = glGetUniformLocation(shaderProg, "objectColor");

    //------------------------------------------------
    // (A) BODY (pink)
    {
        // Pink color: (1.0, 0.4, 0.7)
        glUniform3f(colorLoc, 1.f, 0.4f, 0.7f);
        glm::mat4 body = glm::scale(drone, glm::vec3(1.6f, 0.5f, 1.0f));
        drawCube(body, shaderProg);
    }

    //------------------------------------------------
    // (B) NOSE (yellow, sphere!)
    {
        // We place a small sphere at the front, z=+0.7
        // Color = (1,1,0) => bright yellow
        glUniform3f(colorLoc, 1.f, 1.f, 0.f);
        glm::mat4 nose = glm::translate(drone, glm::vec3(0.f, 0.f, 0.7f));
        // scale the sphere smaller: radius ~0.2
        nose = glm::scale(nose, glm::vec3(0.2f));
        drawSphere(nose, shaderProg);
    }

    //------------------------------------------------
    // (C) ARMS (white)
    auto drawArm = [&](float xOff, float zOff)
    {
        glm::mat4 arm = glm::translate(drone, glm::vec3(xOff, 0.f, zOff));
        arm = glm::scale(arm, glm::vec3(0.7f, 0.1f, 0.1f));
        drawCube(arm, shaderProg);
    };

    glUniform3f(colorLoc, 1.f, 1.f, 1.f);
    drawArm(-0.9f, +0.5f); // front-left
    drawArm(+0.9f, +0.5f); // front-right
    drawArm(-0.9f, -0.5f); // back-left
    drawArm(+0.9f, -0.5f); // back-right

    //------------------------------------------------
    // (D) PROPELLERS (red)
    auto drawPropeller = [&](float xOff, float zOff)
    {
        float propX = xOff + (xOff < 0 ? -0.45f : +0.45f);

        // hub
        glm::mat4 hub = glm::translate(drone, glm::vec3(propX, 0.1f, zOff));
        hub = glm::rotate(hub, propRad, glm::vec3(0,1,0));
        hub = glm::scale(hub, glm::vec3(0.1f));
        drawCube(hub, shaderProg);

        // 4 blades
        for(int i=0; i<4; i++)
        {
            glm::mat4 blade = glm::translate(drone, glm::vec3(propX, 0.1f, zOff));
            blade = glm::rotate(blade, propRad, glm::vec3(0,1,0));
            blade = glm::rotate(blade, glm::radians(90.f * i), glm::vec3(0,1,0));
            blade = glm::translate(blade, glm::vec3(0.f, 0.f, 0.2f));
            blade = glm::scale(blade, glm::vec3(0.05f, 0.02f, 0.35f));
            drawCube(blade, shaderProg);
        }
    };

    // red props
    glUniform3f(colorLoc, 1.f, 0.f, 0.f);
    drawPropeller(-0.9f, +0.5f); 
    drawPropeller(+0.9f, +0.5f); 
    drawPropeller(-0.9f, -0.5f); 
    drawPropeller(+0.9f, -0.5f); 

    //------------------------------------------------
    // (E) LEGS (white)
    auto drawLeg = [&](float xOff, float zOff)
    {
        glm::mat4 leg = glm::translate(drone, glm::vec3(xOff, -0.3f, zOff));
        leg = glm::scale(leg, glm::vec3(0.1f, 0.4f, 0.1f));
        drawCube(leg, shaderProg);
    };

    glUniform3f(colorLoc, 1.f, 1.f, 1.f);
    drawLeg(-0.5f, +0.3f);
    drawLeg(+0.5f, +0.3f);
    drawLeg(-0.5f, -0.3f);
    drawLeg(+0.5f, -0.3f);
}

// ----------------------------------------------------------
// Clean up
void cleanupDrone()
{
    if(gCubeVAO != 0)
    {
        glDeleteVertexArrays(1, &gCubeVAO);
        gCubeVAO = 0;
    }
    if(gSphereVAO != 0)
    {
        glDeleteVertexArrays(1, &gSphereVAO);
        gSphereVAO = 0;
    }
    if(gSphereVBO != 0)
    {
        glDeleteBuffers(1, &gSphereVBO);
        gSphereVBO = 0;
    }
    gDroneGeometryInitialized = false;
}
