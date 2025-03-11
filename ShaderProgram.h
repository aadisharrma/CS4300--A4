#pragma once
#include <glad/glad.h>

/**
 * Given vertex and fragment shader source code, 
 * compile, link, and return the final OpenGL shader program ID.
 */
GLuint createShaderProgram(const char* vertexSrc, const char* fragmentSrc);
