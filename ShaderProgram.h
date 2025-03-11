#pragma once
#include <glad/glad.h>

/**
 * Given vertex and fragment shader source code, compile & link an OpenGL shader program.
 * Returns the program ID (GLuint). On errors, logs to stderr.
 */
GLuint createShaderProgram(const char* vertexSrc, const char* fragmentSrc);
