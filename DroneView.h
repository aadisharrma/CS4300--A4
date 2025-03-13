#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "DroneModel.h"

/**
 * DroneView handles all rendering (VAOs, VBOs, draw calls).
 * It reads from the DroneModel’s data when drawing.
 */
class DroneView
{
public:
    DroneView();
    ~DroneView();

    // Initialize geometry (cube + sphere)
    void initDroneGeometry();

    // Draw the drone, reading data from the model
    void drawDrone(const DroneModel& model, GLuint shaderProg);

    // Cleanup VAOs, VBOs, etc.
    void cleanupDrone();

private:
    // Internal helpers
    void initSphereGeometry();
    void drawCube(const glm::mat4& model, GLuint shaderProg);
    void drawSphere(const glm::mat4& model, GLuint shaderProg);

private:
    // Cube
    GLuint mCubeVAO;
    bool   mDroneGeometryInitialized;

    // Sphere
    GLuint mSphereVAO;
    GLuint mSphereVBO;
    int    mSphereNumVerts;
};
