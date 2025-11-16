#include <IGL/BOX.h>
#include <IGL/auxiliar.h>

#include <GL/glew.h>
#define SOLVE_FGLUT_WARNING
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#define PI 3.141592f

//////////////////////////////////////////////////////////////
// Data stored in CPU memory
//////////////////////////////////////////////////////////////

// Matrices
glm::mat4 proj = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 model = glm::mat4(1.0f);

//////////////////////////////////////////////////////////////
// Variables to access OpenGL Objects
//////////////////////////////////////////////////////////////
// TO BE DEFINED

//////////////////////////////////////////////////////////////
// Auxiliar Functions
//////////////////////////////////////////////////////////////
// TO BE IMPLEMENTED

// CB declaration
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

// Initialization and destruction functions
void initContext(int argc, char **argv);
void initOGL();
void initShader(const char *vname, const char *fname);
void initObj();
void destroy();

// Load the specified shader & return shader ID
// TO BE IMPLEMENTED
GLuint loadShader(const char *fileName, GLenum type);

// Texture creation, configuration and upload to OpenGL
// Return texture ID
// TO BE IMPLEMENTED
unsigned int loadTex(const char *fileName);

int main(int argc, char **argv)
{
#ifdef _WIN32
	std::locale::global(std::locale("spanish")); // Spanish accents
#endif

	initContext(argc, argv);
	initOGL();
	std::string vertexShader = std::string(SHADERS_PATH) + "/shader.v0.vert";
	std::string fragmentShader = std::string(SHADERS_PATH) + "/shader.v0.frag";
	initShader(vertexShader.c_str(), fragmentShader.c_str());
	initObj();

	destroy();

	return 0;
}

//////////////////////////////////////////
// Auxiliar functions
void initContext(int argc, char **argv) {}
void initOGL() {}
void destroy() {}
void initShader(const char *vname, const char *fname) {}
void initObj() {}

GLuint loadShader(const char *fileName, GLenum type) { return 0; }
unsigned int loadTex(const char *fileName) { return 0; }

void renderFunc() {}
void resizeFunc(int width, int height) {}
void idleFunc() {}
void keyboardFunc(unsigned char key, int x, int y) {}
void mouseFunc(int button, int state, int x, int y) {}
