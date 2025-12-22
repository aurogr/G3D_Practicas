#include <GL/glew.h>
#define SOLVE_FGLUT_WARNING
#include <GL/freeglut.h>

#include <iostream>
#include <cstdlib>

#include "helpers\shaderProgram.h"

GLuint ShaderProgram::loadShader(const char *fileName, GLenum type)
{
	unsigned int fileLen;
	char *source = loadStringFromFile(fileName, fileLen);

	//////////////////////////////////////////////
	// Shader creation & compilation
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar **)&source, (const GLint *)&fileLen);
	glCompileShader(shader);
	delete[] source;

	// Check compilation status
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		// Compute error string
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

		char *logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteShader(shader);
		exit(-1);
	}

	return shader;
}


void ShaderProgram::Init(const char *vname, const char *fname, const char* attribs[])
{
    // Create the FBO
	vShader = loadShader(vname, GL_VERTEX_SHADER);
	fShader = loadShader(fname, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vShader);
	glAttachShader(program, fShader);
	
	for (int i = 0; attribs[i] != nullptr; i++) {
		glBindAttribLocation(program, i, attribs[i]);
	}

	/*glBindAttribLocation(program, 0, "inPos");
	glBindAttribLocation(program, 1, "inColor");
	glBindAttribLocation(program, 2, "inNormal");
	glBindAttribLocation(program, 3, "inTexCoord");*/

	glLinkProgram(program);

	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		// Compute error string
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);

		char *logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}
}

void ShaderProgram::Destroy(){
	glDetachShader(program, vShader);
	glDetachShader(program, fShader);
	glDeleteShader(vShader);
	glDeleteShader(fShader);
	glDeleteProgram(program);
}

void ShaderProgram::Use(){
	glUseProgram(program);
}