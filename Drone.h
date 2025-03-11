#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>

void initDroneGeometry();
void drawDrone(float propellerAngle, GLuint shaderProgram);
void cleanupDrone();
