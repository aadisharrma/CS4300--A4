#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

/**
 * Initialize geometry needed by the drone (e.g., the cube VAO).
 */
void initDroneGeometry();

/**
 * Draw the drone with these parameters:
 *  - propAngle: rotation for propellers (in degrees).
 *  - rollAngle: sideways roll (in degrees) from Part 2.
 *  - yaw:       yaw rotation around Y (in degrees).
 *  - pitch:     pitch rotation around X (in degrees).
 *  - position:  world-space position of the drone’s origin.
 *  - shaderProg: current shader program.
 */
void drawDrone(
    float propAngle,
    float rollAngle,
    float yaw,
    float pitch,
    const glm::vec3& position,
    GLuint shaderProg
);

/**
 * Cleanup (delete VAOs, etc.).
 */
void cleanupDrone();
