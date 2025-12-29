/////////////////////////////////////////////////////////////
/////////////////// MOTION BLUR VERSION /////////////////////
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

#include "helpers\framebuffer.h"
#include "helpers\framebuffer.cpp"
#include "helpers\shaderProgram.h"
#include "helpers\shaderProgram.cpp"

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
void initObj();
void initPlane();
void initFBO();
void resizeFBO(unsigned int w, unsigned int h);
void destroy();

// Texture creation, configuration and upload to OpenGL
// Return texture ID
unsigned int loadTex(const char *fileName);

//////////////////////////////////////////////////////////////
// New auxiliar variables
//////////////////////////////////////////////////////////////
FrameBuffer fbo;
ShaderProgram forwardShader;
ShaderProgram postProcessShader;
float motionBlurIntensity = 0.6f;

// camera variables
glm::vec3 COP = glm::vec3(0.0f, 0.0f, 25.0f); // COP is the camera position
glm::vec3 lookAt = glm::vec3(0.0f, 0.0f, -1.0f); // camera orientation
glm::vec3 vUp = glm::vec3(0.0f, 1.0f, 0.0f); // camera's up vector
const float cameraMovementSpeed = 0.5f;
const float cameraRotationSpeed = glm::radians(5.0f);
float previousTime = 0.0f;
//////////////////////////////////////////////////////////////
// New auxiliar functions
//////////////////////////////////////////////////////////////
void setViewMatGivenLookAtAndUp();

//////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
	std::locale::global(std::locale("spanish")); // acentos ;)

	std::cout << "Press 'wasd' (minus) to move camera. Press 'q' or 'e' to rotate it." << std::endl;
	std::cout << "Press '+' or '-' to edit Motion Blur intensity." << std::endl;

	initContext(argc, argv);
	initOGL();
	
	// Forward shader program
	char const* forwardAttribs[] = { "inPos", "inColor", "inNormal", "inTexCoord", nullptr};
	std::string forwardVShaderPath = std::string(SHADERS_PATH) + "/fwRendering.vert";
	std::string forwardFShaderPath = std::string(SHADERS_PATH) + "/fwRendering.frag";
	forwardShader.Init(forwardVShaderPath.c_str(), forwardFShaderPath.c_str(), forwardAttribs);
	// Post-process shader program
	char const* postProcessAttribs[] = { "inPos", nullptr};
	std::string postProcessVShaderPath = std::string(SHADERS_PATH) + "/postProcessing.motionBlur.vert";
	std::string postProcessFShaderPath = std::string(SHADERS_PATH) + "/postProcessing.motionBlur.frag";
	postProcessShader.Init(postProcessVShaderPath.c_str(), postProcessFShaderPath.c_str(), postProcessAttribs);

	initObj();
	initPlane();
	fbo.Init();
	fbo.Resize(SCREEN_SIZE, false, false);

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

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	proj = glm::perspective(glm::radians(60.0f), 1.0f, 1.0f, 50.0f);
	view = glm::mat4(1.0f);
	setViewMatGivenLookAtAndUp();
}

void destroy()
{
	forwardShader.Destroy();
	postProcessShader.Destroy();

	glDeleteBuffers(1, &posVBO);
	glDeleteBuffers(1, &colorVBO);
	glDeleteBuffers(1, &normalVBO);
	glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);

	glDeleteVertexArrays(1, &vao);

	glDeleteBuffers(1, &planeVertexVBO);
	glDeleteVertexArrays(1, &planeVAO);

	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &specularTexId);
	glDeleteTextures(1, &emiTexId);

	fbo.Destroy();
}

void initObj()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &posVBO);
	glBindBuffer(GL_ARRAY_BUFFER, posVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
					cubeVertexPos, GL_STATIC_DRAW);
	glVertexAttribPointer(forwardShader.GetAttribLocation("inPos"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(forwardShader.GetAttribLocation("inPos"));

	glGenBuffers(1, &colorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
					cubeVertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(forwardShader.GetAttribLocation("inColor"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(forwardShader.GetAttribLocation("inColor"));

	glGenBuffers(1, &normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
					cubeVertexNormal, GL_STATIC_DRAW);
	glVertexAttribPointer(forwardShader.GetAttribLocation("inNormal"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(forwardShader.GetAttribLocation("inNormal"));

	glGenBuffers(1, &texCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2,
					cubeVertexTexCoord, GL_STATIC_DRAW);
	glVertexAttribPointer(forwardShader.GetAttribLocation("inTexCoord"), 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(forwardShader.GetAttribLocation("inTexCoord"));

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
	glVertexAttribPointer(postProcessShader.GetAttribLocation("inPos"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(postProcessShader.GetAttribLocation("inPos"));
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
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.idFbo);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	forwardShader.Use();

	// Textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTexId);
	glUniform1i(forwardShader.GetUniformLocation("colorTex"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specularTexId);
	glUniform1i(forwardShader.GetUniformLocation("specularTex"), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, emiTexId);
	glUniform1i(forwardShader.GetUniformLocation("emiTex"), 2);

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

	// ---------- POST PROCESSING ----------
	glEnable(GL_BLEND);
	
	// change last frame intensity based on motionBlurIntensity

	glBlendFunc(GL_ONE_MINUS_CONSTANT_ALPHA, GL_CONSTANT_ALPHA); 
	glBlendColor(0.0f, 0.0f, 0.0f, motionBlurIntensity);
	glBlendEquation(GL_FUNC_ADD);

	// llamar al default framebuffer para pintar el postproceso
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbo.idColorBuffer);
	glUniform1i(postProcessShader.GetUniformLocation("colorTex"), 0);



	// pintar quad
	postProcessShader.Use();
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glDisable(GL_BLEND);


	glutSwapBuffers();
}

void renderCube()
{
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	glUniformMatrix4fv(forwardShader.GetUniformLocation("modelView"), 1, GL_FALSE, &(modelView[0][0]));
	glUniformMatrix4fv(forwardShader.GetUniformLocation("modelViewProj"), 1, GL_FALSE, &(modelViewProj[0][0]));
	glUniformMatrix4fv(forwardShader.GetUniformLocation("normal"), 1, GL_FALSE, &(normal[0][0]));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void *)0);
}

void resizeFunc(int width, int height)
{
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(60.0f), float(width) / float(height), 1.0f, 50.0f);

	fbo.Resize(width, height, false, false);
	glutPostRedisplay();
}

void idleFunc()
{
	// frame rate control
    float currentTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;  // Get elapsed time in seconds
    float deltaTime = currentTime - previousTime;
    if (deltaTime > 0.0f) {
		angle += 1.5f * deltaTime;
		if (angle > PI * 2.0f) {
			angle -= PI * 2.0f;
		}

		previousTime = currentTime;
	}

	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y) {
	// camera movement
	if (key == 'w')
		COP += lookAt * cameraMovementSpeed;
	else if (key == 's')
		COP -= lookAt * cameraMovementSpeed;

	if (key == 'd')
		COP += glm::normalize(glm::cross(lookAt, vUp)) * cameraMovementSpeed;
	else if (key == 'a')
		COP -= glm::normalize(glm::cross(lookAt, vUp)) * cameraMovementSpeed;

	if (key == 'e')
		lookAt = glm::vec3(glm::rotate(glm::mat4(1.0f), -cameraRotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lookAt, 0.0f));
	else if (key == 'q')
		lookAt = glm::vec3(glm::rotate(glm::mat4(1.0f), cameraRotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lookAt, 0.0f));

	setViewMatGivenLookAtAndUp();

	// motion blur intensity control
	if (key == '+') {
		motionBlurIntensity += 0.1f;
		if (motionBlurIntensity > 0.9f) motionBlurIntensity = 0.9f;
		std::cout << "Motion Blur Intensity: " << motionBlurIntensity << std::endl;
	}
	if (key == '-') {
		motionBlurIntensity -= 0.1f;
		if (motionBlurIntensity < 0.1f) motionBlurIntensity = 0.1f;
		std::cout << "Motion Blur Intensity: " << motionBlurIntensity << std::endl;
	}
}

void mouseFunc(int button, int state, int x, int y) {}

void setViewMatGivenLookAtAndUp(){
	glm::vec3 n = -glm::normalize(lookAt);
	glm::vec3 v = glm::normalize(vUp - (n * vUp) * n);
	glm::vec3 u = glm::cross(v, n);

	glm::mat4 cameraView = glm::mat4(glm::vec4(u, 0), glm::vec4(v, 0), glm::vec4(n, 0), glm::vec4(COP, 1));
	view = glm::inverse(cameraView);
}
