#pragma once
#include <glad/glad.h>

/**
 * Initialize geometry needed by the drone (e.g., the cube VAO).
 * Call once at program startup.
 */
void initDroneGeometry();

/**
 * Draw the drone with the specified propeller angle and roll angle.
 *
 * @param propAngle   Degrees of rotation for the propellers (spins continuously).
 * @param rollAngle   Degrees of rotation about the drone’s front/back axis (0..360).
 * @param shaderProg  The currently active shader program ID.
 */
void drawDrone(float propAngle, float rollAngle, GLuint shaderProg);

/**
 * Cleanup drone geometry at program exit (deletes VAOs, etc.).
 */
void cleanupDrone();
