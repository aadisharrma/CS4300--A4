#include "DroneView.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <iostream>

DroneView::DroneView()
    : mCubeVAO(0)
    , mDroneGeometryInitialized(false)
    , mSphereVAO(0)
    , mSphereVBO(0)
    , mSphereNumVerts(0)
{}

DroneView::~DroneView()
{
    cleanupDrone();
}

void DroneView::initSphereGeometry()
{
    if (mSphereVAO != 0) return; // already inited

    const int   stacks = 12;
    const int   slices = 12;
    const float PI     = 3.14159265359f;

    std::vector<float> verts;
    verts.reserve(stacks * slices * 6);

    for(int i = 0; i < stacks; i++)
    {
        float phi0 = PI * (-0.5f + (float)i / stacks);
        float phi1 = PI * (-0.5f + (float)(i+1) / stacks);

        float y0 = std::sin(phi0);
        float r0 = std::cos(phi0);
        float y1 = std::sin(phi1);
        float r1 = std::cos(phi1);

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

    mSphereNumVerts = (int)verts.size() / 3;

    // Create VAO/VBO
    glGenVertexArrays(1, &mSphereVAO);
    glGenBuffers(1, &mSphereVBO);

    glBindVertexArray(mSphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mSphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verts.size(), verts.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void DroneView::initDroneGeometry()
{
    if (mDroneGeometryInitialized) return;

    // 1) Initialize the cube VAO
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
    glGenVertexArrays(1, &mCubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(mCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // 2) Initialize the sphere for the circular nose
    initSphereGeometry();

    mDroneGeometryInitialized = true;
}

void DroneView::drawCube(const glm::mat4& model, GLuint shaderProg)
{
    GLint modelLoc = glGetUniformLocation(shaderProg, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(mCubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void DroneView::drawSphere(const glm::mat4& model, GLuint shaderProg)
{
    GLint modelLoc = glGetUniformLocation(shaderProg, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(mSphereVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, mSphereNumVerts);
    glBindVertexArray(0);
}

void DroneView::drawDrone(const DroneModel& model, GLuint shaderProg)
{
    // Convert angles to radians
    float propRad  = glm::radians(model.getPropAngle());
    float rollRad  = glm::radians(model.getRollAngle());
    float yawRad   = glm::radians(model.getYaw());
    float pitchRad = glm::radians(model.getPitch());

    // Base transform
    glm::mat4 drone(1.0f);
    drone = glm::translate(drone, model.getPosition());
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
        glUniform3f(colorLoc, 1.f, 0.4f, 0.7f);
        glm::mat4 body = glm::scale(drone, glm::vec3(1.6f, 0.5f, 1.0f));
        drawCube(body, shaderProg);
    }

    //------------------------------------------------
    // (B) NOSE (yellow, sphere)
    {
        glUniform3f(colorLoc, 1.f, 1.f, 0.f);
        glm::mat4 nose = glm::translate(drone, glm::vec3(0.f, 0.f, 0.7f));
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
        for(int i = 0; i < 4; i++)
        {
            glm::mat4 blade = glm::translate(drone, glm::vec3(propX, 0.1f, zOff));
            blade = glm::rotate(blade, propRad, glm::vec3(0,1,0));
            blade = glm::rotate(blade, glm::radians(90.f * i), glm::vec3(0,1,0));
            blade = glm::translate(blade, glm::vec3(0.f, 0.f, 0.2f));
            blade = glm::scale(blade, glm::vec3(0.05f, 0.02f, 0.35f));
            drawCube(blade, shaderProg);
        }
    };

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

void DroneView::cleanupDrone()
{
    if(mCubeVAO != 0)
    {
        glDeleteVertexArrays(1, &mCubeVAO);
        mCubeVAO = 0;
    }
    if(mSphereVAO != 0)
    {
        glDeleteVertexArrays(1, &mSphereVAO);
        mSphereVAO = 0;
    }
    if(mSphereVBO != 0)
    {
        glDeleteBuffers(1, &mSphereVBO);
        mSphereVBO = 0;
    }

    mDroneGeometryInitialized = false;
}
