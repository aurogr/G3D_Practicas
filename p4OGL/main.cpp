/////////////////////////////////////////////////////////////
/////////////////// DEPTH OF FIELD VERSION //////////////////
/////////////////////////////////////////////////////////////
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
#include <vector>

#define PI 3.141592f
#define RAND_SEED 31415926
#define SCREEN_SIZE 500, 500



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
unsigned int vshader;
unsigned int fshader;
unsigned int program;

unsigned int postProcessVShader; 
unsigned int postProcessFShader; 
unsigned int postProcessProgram;

// Uniform matrix ID variables
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;

// Uniform texture ID variables
int uColorTex;
int uSpecularTex;
int uEmiTex;
unsigned int uColorTexPP;
unsigned int uDepthTexPP;

// Attributes
int inPos;
int inColor;
int inNormal;
int inTexCoord;
int inPosPP;

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
void initShaderFw(const char *vname, const char *fname);
void initShaderPP(const char *vname, const char *fname);
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
 
// CONVOLUTION MASKS
struct ConvolutionMask {
	int size;
	std::vector<glm::vec2> texIdx;
	std::vector<float> mask;
};

std::vector<ConvolutionMask> cMasks;
unsigned int mask = 0; // idx mask in use

//////////////////////////////////////////////////////////////
// New auxiliar functions
//////////////////////////////////////////////////////////////
void initConvMasks();

int main(int argc, char **argv)
{
	std::locale::global(std::locale("spanish")); // acentos ;)

	initContext(argc, argv);
	initOGL();
	std::string vertexShader = std::string(SHADERS_PATH) + "/fwRendering.v1.vert";
	std::string fragmentShader = std::string(SHADERS_PATH) + "/fwRendering.v1.frag";
	initShaderFw(vertexShader.c_str(), fragmentShader.c_str());
	vertexShader = std::string(SHADERS_PATH) + "/postProcessing.v2.vert";
	fragmentShader = std::string(SHADERS_PATH) + "/postProcessing.v2.frag";
	initShaderPP(vertexShader.c_str(), fragmentShader.c_str());
	initObj();
	initPlane();
	initConvMasks();
	initFBO();
	resizeFBO(SCREEN_SIZE);

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
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);

	glDetachShader(postProcessProgram, postProcessVShader);
	glDetachShader(postProcessProgram, postProcessFShader);
	glDeleteShader(postProcessVShader);
	glDeleteShader(postProcessFShader);
	glDeleteProgram(postProcessProgram);

	if (inPos != -1)
		glDeleteBuffers(1, &posVBO);
	if (inColor != -1)
		glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1)
		glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1)
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
}

void initShaderFw(const char *vname, const char *fname)
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

	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

	uColorTex = glGetUniformLocation(program, "colorTex");
	uSpecularTex = glGetUniformLocation(program, "specularTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");

	inPos = glGetAttribLocation(program, "inPos");
	inColor = glGetAttribLocation(program, "inColor");
	inNormal = glGetAttribLocation(program, "inNormal");
	inTexCoord = glGetAttribLocation(program, "inTexCoord");
}

void initShaderPP(const char* vname, const char* fname) 
{
	postProcessVShader = loadShader(vname, GL_VERTEX_SHADER);
	postProcessFShader = loadShader(fname, GL_FRAGMENT_SHADER);
	postProcessProgram = glCreateProgram();

	glAttachShader(postProcessProgram, postProcessVShader);
	glAttachShader(postProcessProgram, postProcessFShader);
	glBindAttribLocation(postProcessProgram, 0, "inPos");

	glLinkProgram(postProcessProgram);

	int linked;
	glGetProgramiv(postProcessProgram, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(postProcessProgram, GL_INFO_LOG_LENGTH, &logLen);

		char* logString = new char[logLen];
		glGetProgramInfoLog(postProcessProgram, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete logString;

		glDeleteProgram(postProcessProgram);
		postProcessProgram = 0;
		exit(-1);
	}

	uColorTexPP = glGetUniformLocation(postProcessProgram, "colorTex");
	inPosPP = glGetAttribLocation(postProcessProgram, "inPos");
	uDepthTexPP = glGetUniformLocation(postProcessProgram, "depthTex");

	glGetUniformLocation(postProcessProgram, "convMask");
	glGetUniformLocation(postProcessProgram, "convMaskSize");
	glGetUniformLocation(postProcessProgram, "convTexId");
}

void initObj()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		glGenBuffers(1, &posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
					 cubeVertexPos, GL_STATIC_DRAW);
		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}

	if (inColor != -1)
	{
		glGenBuffers(1, &colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
					 cubeVertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}

	if (inNormal != -1)
	{
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
					 cubeVertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal);
	}

	if (inTexCoord != -1)
	{
		glGenBuffers(1, &texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2,
					 cubeVertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord);
	}

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
	glVertexAttribPointer(inPosPP, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(inPosPP);
}

void initFBO() 
{
	glGenFramebuffers(1, &fbo);
	glGenTextures(1, &colorBuffTexId);
	glGenTextures(1, &depthBuffTexId);
}

void resizeFBO(unsigned int w, unsigned int h) 
{
	glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffTexId, 0);
	
	const GLenum buffs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, buffs);

	if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
		std::cerr << "Error configurando el FBO" << std::endl;
		exit(-1);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	// Textures
	if (uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0);
	}

	if (uSpecularTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, specularTexId);
		glUniform1i(uSpecularTex, 1);
	}

	if (uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 2);
	}

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

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(postProcessProgram);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	if (uColorTexPP != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
		glUniform1i(uColorTexPP, 0);
	}
	if (uDepthTexPP != -1) {
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
		glUniform1i(uDepthTexPP, 1);
	}
	
	// Convolution Mask assignment
	glUniform1i(glGetUniformLocation(postProcessProgram, "convMaskSize"), cMasks[mask].size);
	glUniform1fv(glGetUniformLocation(postProcessProgram, "convMask"), cMasks[mask].mask.size(), cMasks[mask].mask.data());
	glUniform2fv(glGetUniformLocation(postProcessProgram, "convMaskIdx"), cMasks[mask].texIdx.size(), &cMasks[mask].texIdx[0].x);

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

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void *)0);
}

void resizeFunc(int width, int height)
{
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(60.0f), float(width) / float(height), 1.0f, 50.0f);

	resizeFBO(width, height);
	glutPostRedisplay();
}

void idleFunc()
{
	angle = (angle > PI * 2.0f) ? 0 : angle + 0.0002f;

	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y) 
{
	if (key == 'c' || key == 'C') {
		mask += 1;
		if (mask >= cMasks.size()) mask = 0;
		std::cout << "Conv mask: " << mask << std::endl;

	}
}
void mouseFunc(int button, int state, int x, int y) {}

void initConvMasks() 
{
	// 3x3
	ConvolutionMask mask3x3;
	mask3x3.size = 9;
	float factor = float(1.0 / 14.0);
	mask3x3.mask = {
		1.0f * factor, 2.0f * factor, 1.0f * factor,
		2.0f * factor, 2.0f * factor, 2.0f * factor,
		1.0f * factor, 2.0f * factor, 1.0f * factor
	};
	mask3x3.texIdx = {
		glm::vec2(-1.0f,  1.0f), glm::vec2(0.0f,  1.0f), glm::vec2(1.0f,  1.0f),
		glm::vec2(-1.0f,  0.0f), glm::vec2(0.0f,  0.0f), glm::vec2(1.0f,  0.0f),
		glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, -1.0f), glm::vec2(1.0f, -1.0f)
	};
	cMasks.push_back(mask3x3);

	// 5x5
	ConvolutionMask mask5x5;
	mask5x5.size = 25;
	factor = 1.0f / 65.0f;
	mask5x5.mask = {
		1 * factor, 2 * factor, 3 * factor, 2 * factor, 1 * factor,
		2 * factor, 3 * factor, 4 * factor, 3 * factor, 2 * factor,
		3 * factor, 4 * factor, 5 * factor, 4 * factor, 3 * factor,
		2 * factor, 3 * factor, 4 * factor, 3 * factor, 2 * factor,
		1 * factor, 2 * factor, 3 * factor, 2 * factor, 1 * factor
	};
	mask5x5.texIdx = {
		glm::vec2(-2, 2), glm::vec2(-1, 2), glm::vec2(0, 2), glm::vec2(1, 2), glm::vec2(2, 2),
		glm::vec2(-2, 1), glm::vec2(-1, 1), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(2, 1),
		glm::vec2(-2, 0), glm::vec2(-1, 0), glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(2, 0),
		glm::vec2(-2, -1), glm::vec2(-1, -1), glm::vec2(0, -1), glm::vec2(1, -1), glm::vec2(2, -1),
		glm::vec2(-2, -2), glm::vec2(-1, -2), glm::vec2(0, -2), glm::vec2(1, -2), glm::vec2(2, -2)
	};
	cMasks.push_back(mask5x5);

	// 3x3
	ConvolutionMask maskSharpen;
	maskSharpen.size = 9;
	factor = float(1.0/ 2.0);
	maskSharpen.mask = {
		0.0f * factor, -1.0f * factor, 0.0f * factor,
		-1.0f * factor, 5.0f * factor, -1.0f * factor,
		0.0f * factor, -1.0f * factor, 0.0f * factor
	};
	maskSharpen.texIdx = {
		glm::vec2(-1.0f,  1.0f), glm::vec2(0.0f,  1.0f), glm::vec2(1.0f,  1.0f),
		glm::vec2(-1.0f,  0.0f), glm::vec2(0.0f,  0.0f), glm::vec2(1.0f,  0.0f),
		glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, -1.0f), glm::vec2(1.0f, -1.0f)
	};
	cMasks.push_back(maskSharpen);
}