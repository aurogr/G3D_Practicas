#include <IGL/BOX.h>
#include <IGL/PLANE.h>
#include <IGL/auxiliar.h>

#include <GL/glew.h>
#define SOLVE_FGLUT_WARNING
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cstdlib>

#include "gbuffer.h"
#include "gbuffer.cpp"

#define PI 3.141592f
#define RAND_SEED 31415926
#define SCREEN_SIZE 500, 500

unsigned int gFbo;
unsigned int gPosition;
unsigned int gNormal;
unsigned int gAlbedoSpec;
unsigned int gDepth;
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
float angle = 0.0f;

// VAO
unsigned int vao;

// VBOs that are part of the object
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

unsigned int colorTexId;
unsigned int specularTexId;
unsigned int emiTexId;

// plane VAO
unsigned int planeVAO;
unsigned int planeVertexVBO;

// plane FBO
unsigned int fbo;
unsigned int colorBuffTexId;
unsigned int depthBuffTexId;

// TO BE DEFINED
unsigned int dSGeomVShader;
unsigned int dSGeomFShader;
unsigned int dsLightVShader; 
unsigned int dsLightFShader; 
unsigned int dsGeomProgram;
unsigned int dsLightProgram;

//////////////////////////////////////////////////////////////
// Auxiliar functions
//////////////////////////////////////////////////////////////

// CB declaration
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

void renderCube();

// Initialization and destruction functions
void initContext(int argc, char **argv);
void initOGL();
void initShader(const char *vname, const char *fname, unsigned int &program, unsigned int &vshader, unsigned int &fshader);
void initObj();
void initPlane();
void initFBO();
void resizeFBO(unsigned int w, unsigned int h);
void destroy();

// Load the specified shader & return shader ID
// TO BE IMPLEMENTED
GLuint loadShader(const char *fileName, GLenum type);

// Texture creation, configuration and upload to OpenGL
// Return texture ID
// TO BE IMPLEMENTED
unsigned int loadTex(const char *fileName);

//////////////////////////////////////////////////////////////
// New auxiliar variables
//////////////////////////////////////////////////////////////
GBuffer gBuffer;
//////////////////////////////////////////////////////////////
// New auxiliar functions
//////////////////////////////////////////////////////////////
// TO BE IMPLEMENTED

int main(int argc, char **argv)
{
	std::locale::global(std::locale("spanish")); // acentos ;)

	initContext(argc, argv);
	initOGL();
	// Geometry shader
	initShader((std::string(SHADERS_PATH) + "/deferred.Geometry.vert").c_str(), (std::string(SHADERS_PATH) + "/deferred.Geometry.frag").c_str(), dsGeomProgram, dSGeomVShader, dSGeomFShader);
	initShader((std::string(SHADERS_PATH) + "/deferred.Lighting.vert").c_str(), (std::string(SHADERS_PATH) + "/deferred.Lighting.frag").c_str(), dsLightProgram, dsLightVShader, dsLightFShader);
	initObj();
	initPlane();
	gBuffer.Init();
	gBuffer.Resize(SCREEN_SIZE);

	glutMainLoop();

	destroy();

	return 0;
}

//////////////////////////////////////////
// Auxiliar functions
void initContext(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	// glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(SCREEN_SIZE);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("G3D Exercises");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}

	const GLubyte *oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
}

void initOGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	proj = glm::perspective(glm::radians(60.0f), 1.0f, 1.0f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -25.0f;
}

void destroy()
{
	glDetachShader(dsGeomProgram, dSGeomVShader);
	glDetachShader(dsGeomProgram, dSGeomFShader);
	glDeleteShader(dSGeomVShader);
	glDeleteShader(dSGeomFShader);
	glDeleteProgram(dsGeomProgram);

	glDetachShader(dsGeomProgram, dsLightVShader);
	glDetachShader(dsLightProgram, dsLightFShader);
	glDeleteShader(dsLightVShader);
	glDeleteShader(dsLightFShader);
	glDeleteProgram(dsLightProgram);

	glDeleteBuffers(1, &posVBO);
	glDeleteBuffers(1, &colorVBO);
	glDeleteBuffers(1, &normalVBO);
	glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);

	glDeleteVertexArrays(1, &vao);

	glDeleteBuffers(1, &planeVertexVBO);
	glDeleteVertexArrays(1, &planeVAO);

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &colorBuffTexId);
	glDeleteTextures(1, &depthBuffTexId);

	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &specularTexId);
	glDeleteTextures(1, &emiTexId);

	gBuffer.Destroy();

}

void initShader(const char *vname, const char *fname, unsigned int &program, unsigned int &vshader, unsigned int &fshader)
{
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);

	glBindAttribLocation(program, 0, "inPos");
	glBindAttribLocation(program, 1, "inColor");
	glBindAttribLocation(program, 2, "inNormal");
	glBindAttribLocation(program, 3, "inTexCoord");

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

void initObj()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &posVBO);
	glBindBuffer(GL_ARRAY_BUFFER, posVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
					cubeVertexPos, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(dsGeomProgram, "inPos"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(glGetAttribLocation(dsGeomProgram, "inPos"));

	glGenBuffers(1, &colorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
					cubeVertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(dsGeomProgram, "inColor"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(glGetAttribLocation(dsGeomProgram, "inColor"));

	glGenBuffers(1, &normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
					cubeVertexNormal, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(dsGeomProgram, "inNormal"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(glGetAttribLocation(dsGeomProgram, "inNormal"));
	
	glGenBuffers(1, &texCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2,
					cubeVertexTexCoord, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(dsGeomProgram, "inTexCoord"), 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(glGetAttribLocation(dsGeomProgram, "inTexCoord"));

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 cubeNTriangleIndex * sizeof(unsigned int) * 3, cubeTriangleIndex,
				 GL_STATIC_DRAW);

	model = glm::mat4(1.0f);

	std::string albedoTexPath = std::string(TEXTURES_PATH) + "/color2.png";
	std::string specularTexPath = std::string(TEXTURES_PATH) + "/specMap.png";
	std::string emissiveTexPath = std::string(TEXTURES_PATH) + "/emissive.png";
	colorTexId = loadTex(albedoTexPath.c_str());
	specularTexId = loadTex(specularTexPath.c_str());
	emiTexId = loadTex(emissiveTexPath.c_str());
}

void initPlane() 
{
	glGenVertexArrays(1, &planeVAO);
	glBindVertexArray(planeVAO);

	glGenBuffers(1, &planeVertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVertexVBO);
	glBufferData(GL_ARRAY_BUFFER, planeNVertex * sizeof(float) * 3,
		planeVertexPos, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(dsLightProgram, "inPos"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(glGetAttribLocation(dsLightProgram, "inPos"));
}

GLuint loadShader(const char *fileName, GLenum type)
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

unsigned int loadTex(const char *fileName)
{
	unsigned char *map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);

	if (!map)
	{
		std::cout << "Error loading file: " << fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, (GLvoid *)map);
	delete[] map;
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	return texId;
}

void renderFunc()
{
	// ---------- GEOMETRY PASS ----------
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.gFbo);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(dsGeomProgram);
    // Textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTexId);
	glUniform1i(glGetUniformLocation(dsGeomProgram, "colorTex"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specularTexId);
	glUniform1i(glGetUniformLocation(dsGeomProgram, "specularTex"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, emiTexId);
	glUniform1i(glGetUniformLocation(dsGeomProgram, "emiTex"), 2);

	// render cubes
	model = glm::mat4(2.0f);
	model[3].w = 1.0f;
	model = glm::rotate(model, angle, glm::vec3(1.0f, 1.0f, 0.0f));
	renderCube();
	
	std::srand(RAND_SEED);
	for (unsigned int i = 0; i < 10; i++)
	{
		float size = float(std::rand() % 3 + 1);

		glm::vec3 axis(glm::vec3(float(std::rand() % 2),
								 float(std::rand() % 2), float(std::rand() % 2)));
		if (glm::all(glm::equal(axis, glm::vec3(0.0f))))
			axis = glm::vec3(1.0f);

		float trans = float(std::rand() % 7 + 3) * 1.00f + 0.5f;
		glm::vec3 transVec = axis * trans;
		transVec.x *= (std::rand() % 2) ? 1.0f : -1.0f;
		transVec.y *= (std::rand() % 2) ? 1.0f : -1.0f;
		transVec.z *= (std::rand() % 2) ? 1.0f : -1.0f;

		model = glm::rotate(glm::mat4(1.0f), angle * 2.0f * size, axis);
		model = glm::translate(model, transVec);
		model = glm::rotate(model, angle * 2.0f * size, axis);
		model = glm::scale(model, glm::vec3(1.0f / (size * 0.7f)));
		renderCube();
	}
	
	
	// ---------- LIGHTING PASS ----------
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(dsLightProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition);
	glUniform1i(glGetUniformLocation(dsLightProgram, "gPosition"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gNormal);
	glUniform1i(glGetUniformLocation(dsLightProgram, "gNormal"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gAlbedoSpec);
	glUniform1i(glGetUniformLocation(dsLightProgram, "gAlbedoSpec"), 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gEmissive);
	glUniform1i(glGetUniformLocation(dsLightProgram, "gEmissive"), 3);


	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	// send light uniforms
	// ....

	// render plane
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	
	glutSwapBuffers();
	
}

void renderCube()
{
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	glUniformMatrix4fv(glGetUniformLocation(dsGeomProgram, "modelView"), 1, GL_FALSE, &(modelView[0][0]));
	glUniformMatrix4fv(glGetUniformLocation(dsGeomProgram, "modelViewProj"), 1, GL_FALSE, &(modelViewProj[0][0]));
	glUniformMatrix4fv(glGetUniformLocation(dsGeomProgram, "normal"), 1, GL_FALSE, &(normal[0][0]));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void *)0);
}

void resizeFunc(int width, int height)
{
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(60.0f), float(width) / float(height), 1.0f, 50.0f);

	gBuffer.Resize(width, height);
	glutPostRedisplay();
}

void idleFunc()
{
	angle = (angle > PI * 2.0f) ? 0 : angle + 0.0002f;

	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y) {}
void mouseFunc(int button, int state, int x, int y) {}
