#include <IGL/BOX.h>
#include <IGL/auxiliar.h>

#include <GL/glew.h>
#define SOLVE_FGLUT_WARNING
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define PI 3.141592f

//////////////////////////////////////////////////////////////
// Data stored in CPU memory
//////////////////////////////////////////////////////////////

// Matrices
glm::mat4 proj = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 model2 = glm::mat4(1.0f);
glm::mat4 model3 = glm::mat4(1.0f);

//////////////////////////////////////////////////////////////
// Variables to access OpenGL Objects
//////////////////////////////////////////////////////////////

/// PROGRAM 1 ///
unsigned int vshader;
unsigned int fshader;
unsigned int program;		// 1 program = 1 pipeline configuration = 1 vertex shader + 1 fragment shader

//Uniform Variables
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;
// - Uniform Textures
int uColorTex;
int uEmiTex;
int uNormalTex;
int uSpecTex;
// - Textures
unsigned int colorTexId;
unsigned int emiTexId;
unsigned int normalTexId;
unsigned int specTexId;


/// PROGRAM 2 ///
unsigned int vshader2;
unsigned int fshader2;
unsigned int program2;

//Uniform Variables
int uModelViewMat2;
int uModelViewProjMat2;
int uNormalMat2;
// - Uniform Textures
int uColorTex2;
int uEmiTex2;
int uNormalTex2;
int uSpecTex2;

//Attributes
unsigned int obj3TrianglesN;

//VAO. 1 VAO for each object. Each VAO has multiple VBO
unsigned int vao;

//VBOs for each VAO. Store different types of attributes for the object.
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;
unsigned int tangentVBO;

//VAO. 3 VAO for each object. Each VAO has multiple VBO
unsigned int vao3;

//VBOs for each VAO. Store different types of attributes for the object.
unsigned int posVBO3;
unsigned int colorVBO3;
unsigned int normalVBO3;
unsigned int texCoordVBO3;
unsigned int triangleIndexVBO3;
unsigned int tangentVBO3;
//////////////////////////////////////////////////////////////
// New auxiliar variables

//Light attributes
int inLightPos;
int inLightIa;
int inLightId;
int inLightIs;

glm::vec3 lightPos = glm::vec3(5.0, 0.0, 0.0);
glm::vec3 lightIa = glm::vec3(0.3, 0.3, 0.3);
glm::vec3 lightId = glm::vec3(1.0, 1.0, 1.0);
glm::vec3 lightIs = glm::vec3(1.0, 1.0, 1.0);

int lightKeyboardSetting = 1; // 1 = Ia
							  // 2 = Id
							  // 3 = Is

// camera variables
glm::vec3 COP = glm::vec3(0.0f, 0.0f, 6.0f); // COP is the camera position
glm::vec3 lookAt = glm::vec3(0.0f, 0.0f, -1.0f); // camera orientation
glm::vec3 vUp = glm::vec3(0.0f, 1.0f, 0.0f); // camera's up vector
const float cameraMovementSpeed = 0.3f;
const float cameraRotationSpeed = glm::radians(10.0f);
const float cameraYawPitchSpeed = 0.05f;
int lastXmouse = 0;
int lastYmouse = 0;

int mainBifurcations = 0; // 0: cube spinning (better for shaders that use textures)
						  // 1: cube visualized as a plane for better anisotropic filtering visualization

// Anisotropic filtering
bool anistropicFilterOn = true;
GLfloat fLargest;

// bezier movement
float t = 0.0f;

//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// Auxiliar Functions
void setViewMatGivenLookAtAndUp();
void assimpModelLoad();
void bezierMovement();
//////////////////////////////////////////////////////////////
// TO BE IMPLEMENTED

// CallBack declaration
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);
void mouseMotionFunc(int x, int y);

// Initialization and destruction functions
void initContext(int argc, char** argv);
void initOGL();
void initShader1(const char* vname, const char* fname);
void initShader2(const char* vname, const char* fname);
void initCube();
void initAssimp();
void initTextures();
void destroy();

// Load the specified shader & return shader ID
GLuint loadShader(const char* fileName, GLenum type);

// Texture creation, configuration and upload to OpenGL. Return texture ID
unsigned int loadTex(const char* fileName);

int main(int argc, char** argv)
{
#ifdef _WIN32
	std::locale::global(std::locale("spanish")); // Spanish accents
#endif
	if (mainBifurcations == 1)
		anistropicFilterOn = true;

	// Anisotropic filtering
	if (anistropicFilterOn){
		
		if(glewIsSupported("GL_EXT_texture_filter_anisotropic")){
			anistropicFilterOn = false;
			std::cout << "Anisotropic filtering is not supported. It has been disabled for the execution." << std::endl;
		} else
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest); // get maximum amount of anisotropy supported
	}

	initContext(argc, argv);
	initOGL();
	//std::string vertexShader = std::string(SHADERS_PATH) + "/shader.v1.vert";
	std::string vertexShader = std::string(SHADERS_PATH) + "/shader.op2_bumpMapping.vert";
	//std::string fragmentShader = std::string(SHADERS_PATH) + "/shader.v1.frag";
	std::string fragmentShader = std::string(SHADERS_PATH) + "/shader.op2_bumpMapping.frag";
	initShader1(vertexShader.c_str(), fragmentShader.c_str());
	vertexShader = std::string(SHADERS_PATH) + "/shader.v1.vert";
	fragmentShader = std::string(SHADERS_PATH) + "/shader.v1.frag";
	initShader2(vertexShader.c_str(), fragmentShader.c_str());
	initTextures();
	initCube();
	initAssimp();

	glutMainLoop();

	destroy();

	return 0;
}

//////////////////////////////////////////
// Auxiliar functions
void initContext(int argc, char** argv)
{
	// initialize context using GLUT
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// define Frame Buffer and create window
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // two frame buffers (they swap each frame), one depth buffer
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Pr cticas OGL");

	// extensions
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}
	const GLubyte* oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	// relate each glut function to OpenGL event
	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(mouseMotionFunc);
}

void initOGL()
{
	glEnable(GL_DEPTH_TEST);										// activate Z-test
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);			// define bg color
	glFrontFace(GL_CCW);											// front face orientation (clockwise = right hand, counter clockwise = left hand)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);					// configure rasterization
	glEnable(GL_CULL_FACE);											// activate culling

	// define matrices
	proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -6;
}

void destroy()
{
	// program 1
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);

	// 1. VAO VBO
	glDeleteBuffers(1, &posVBO);
	glDeleteBuffers(1, &colorVBO);
	glDeleteBuffers(1, &normalVBO);
	glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &tangentVBO);
	glDeleteBuffers(1, &triangleIndexVBO);
	glDeleteVertexArrays(1, &vao);

	// textures
	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);
	glDeleteTextures(1, &normalTexId);
	glDeleteTextures(1, &specTexId);

	// program 2	
	glDetachShader(program2, vshader2);
	glDetachShader(program2, fshader2);
	glDeleteShader(vshader2);
	glDeleteShader(fshader2);
	glDeleteProgram(program2);

	// 2. VAO VBO
	glDeleteBuffers(1, &posVBO3);
	glDeleteBuffers(1, &colorVBO3);
	glDeleteBuffers(1, &normalVBO3);
	glDeleteBuffers(1, &texCoordVBO3);
	glDeleteBuffers(1, &tangentVBO3);
	glDeleteBuffers(1, &triangleIndexVBO3);
	glDeleteVertexArrays(1, &vao3);
}

void initShader1(const char* vname, const char* fname)
{
	// create vertex and fragment shader
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	// link shaders to a program
	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);

	// bind each program attribute to a socket, before linking program
	glBindAttribLocation(program, 0, "inPos");
	glBindAttribLocation(program, 1, "inColor");
	glBindAttribLocation(program, 2, "inNormal");
	glBindAttribLocation(program, 3, "inTexCoord");
	glBindAttribLocation(program, 4, "inTangent");

	glLinkProgram(program);

	// check for linking errors 
	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Generate error string
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}

	// idx for uniform variables
	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");
	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");
	uNormalTex = glGetUniformLocation(program, "normalTex");
	uSpecTex = glGetUniformLocation(program, "specularTex");
	// idx for light attributes
	inLightPos = glGetUniformLocation(program, "inLightPos");	
	inLightIa = glGetUniformLocation(program, "inLightIa");	
	inLightId = glGetUniformLocation(program, "inLightId");
	inLightIs = glGetUniformLocation(program, "inLightIs");
}

void initShader2(const char* vname, const char* fname)
{
	// create vertex and fragment shader
	vshader2 = loadShader(vname, GL_VERTEX_SHADER);
	fshader2 = loadShader(fname, GL_FRAGMENT_SHADER);

	// link shaders to a program
	program2 = glCreateProgram();
	glAttachShader(program2, vshader2);
	glAttachShader(program2, fshader2);

	// bind each program attribute to a socket, before linking program
	glBindAttribLocation(program2, 0, "inPos");
	glBindAttribLocation(program2, 1, "inColor");
	glBindAttribLocation(program2, 2, "inNormal");
	glBindAttribLocation(program2, 3, "inTexCoord");
	glBindAttribLocation(program2, 4, "inTangent");

	glLinkProgram(program2);

	// check for linking errors 
	int linked;
	glGetProgramiv(program2, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Generate error string
		GLint logLen;
		glGetProgramiv(program2, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(program2, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteProgram(program2);
		program2 = 0;
		exit(-1);
	}

	// idx for uniform variables
	uNormalMat2 = glGetUniformLocation(program2, "normal");
	uModelViewMat2 = glGetUniformLocation(program2, "modelView");
	uModelViewProjMat2 = glGetUniformLocation(program2, "modelViewProj");
	uColorTex2 = glGetUniformLocation(program2, "colorTex");
	uEmiTex2 = glGetUniformLocation(program2, "emiTex");
	uNormalTex2 = glGetUniformLocation(program2, "normalTex");
	uSpecTex2 = glGetUniformLocation(program2, "specularTex");
	// idx for light attributes
	inLightPos = glGetUniformLocation(program2, "inLightPos");	
	inLightIa = glGetUniformLocation(program2, "inLightIa");	
	inLightId = glGetUniformLocation(program2, "inLightId");
	inLightIs = glGetUniformLocation(program2, "inLightIs");
}

void initCube()
{
	// create and activate VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// create and configure mesh attributes
	glGenBuffers(1, &posVBO);
	glBindBuffer(GL_ARRAY_BUFFER, posVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
		cubeVertexPos, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	
	glGenBuffers(1, &colorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
		cubeVertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
		cubeVertexNormal, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	
	glGenBuffers(1, &texCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2,
		cubeVertexTexCoord, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);
	
	glGenBuffers(1, &tangentVBO);
	glBindBuffer(GL_ARRAY_BUFFER, tangentVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
		cubeVertexTangent, GL_STATIC_DRAW);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(4);

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeNTriangleIndex * sizeof(unsigned int) * 3,
		cubeTriangleIndex, GL_STATIC_DRAW);

	// model matrix for this object
	model = glm::mat4(1.0f);
}

void initTextures() 
{
	std::string colorTex = std::string(TEXTURES_PATH) + "/color2.png";
	std::string emissiveTex = std::string(TEXTURES_PATH) + "/emissive.png";
	std::string normalTex = std::string(TEXTURES_PATH) + "/normal.png";
	std::string specularTex = std::string(TEXTURES_PATH) + "/specMap.png";
	colorTexId = loadTex(colorTex.c_str());
	emiTexId = loadTex(emissiveTex.c_str());
	normalTexId = loadTex(normalTex.c_str());
	specTexId = loadTex(specularTex.c_str());
}


GLuint loadShader(const char* fileName, GLenum type)
{
	unsigned int fileLen;
	char* source = loadStringFromFile(fileName, fileLen);
	//////////////////////////////////////////////
	//Shader creation and compilation
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1, 
		(const GLchar**)&source, (const GLint*)&fileLen);
	glCompileShader(shader);
	delete[] source;

	//Check for compilation errors
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Generate error string
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteShader(shader);
		exit(-1);
	}

	return shader; // compiled shader idx
}
unsigned int loadTex(const char* fileName)
{
	//load texture from file
	unsigned char* map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);
	//check load errors
	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	// create texture idx, activate and upload GPU
	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)map); // texture configuration for GPU
	delete[] map; // delete in CPU

	// create texture mipmaps, define access mode
	glGenerateMipmap(GL_TEXTURE_2D);

	if (anistropicFilterOn){
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest); // anistropic filtering
	} else{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // for zoom out, trilinear interpolation
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);				// for zoom in, bilinear interpolation
	}
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);	
	
	return texId; 
}

void renderFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// CUBE 1
	glUseProgram(program);

	// generate light attributes for render
	if (inLightPos != -1)
	{
		glm::vec3 lightPosView = glm::vec3(view * glm::vec4(lightPos, 1));
		glUniform3fv(inLightPos, 1, &lightPosView[0]);
	}
	if (inLightIa != -1)
	{
		glUniform3fv(inLightIa, 1, &lightIa[0]);
	}
	if (inLightId != -1)
	{
		
		glUniform3fv(inLightId, 1, &lightId[0]);
	}
	if (inLightIs != -1)
	{
		
		glUniform3fv(inLightIs, 1, &lightIs[0]);
	}

	// generate matrices for v shader
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));
	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
			&(normal[0][0]));

	// activate VAO with object configuration
	glBindVertexArray(vao);
	// draw triangles
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3,
		GL_UNSIGNED_INT, (void*)0);

	//Activate textures, bind to active program
	if (uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0);
	}
	if (uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1);
	}if (uNormalTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, normalTexId);
		glUniform1i(uNormalTex, 2);
	}
	if (uSpecTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_2D, specTexId);
		glUniform1i(uSpecTex, 3);
	}

	// activate VAO with object configuration
	glBindVertexArray(vao);
	// draw triangles
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

	// CUBE 2
	glUseProgram(program2);

	if (inLightPos != -1)
	{
		glm::vec3 lightPosView = glm::vec3(view * glm::vec4(lightPos, 1));
		glUniform3fv(inLightPos, 1, &lightPosView[0]);
	}
	if (inLightIa != -1)
	{
		glUniform3fv(inLightIa, 1, &lightIa[0]);
	}
	if (inLightId != -1)
	{
		
		glUniform3fv(inLightId, 1, &lightId[0]);
	}
	if (inLightIs != -1)
	{
		
		glUniform3fv(inLightIs, 1, &lightIs[0]);
	}

	// generate matrices for v shader
	modelView = view * model2;
	modelViewProj = proj * view * model2;
	normal = glm::transpose(glm::inverse(modelView));
	if (uModelViewMat2 != -1)
		glUniformMatrix4fv(uModelViewMat2, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat2 != -1)
		glUniformMatrix4fv(uModelViewProjMat2, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat2 != -1)
		glUniformMatrix4fv(uNormalMat2, 1, GL_FALSE, &(normal[0][0]));

	//Activate textures, bind to active program
	if (uColorTex2 != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex2, 0);
	}
	if (uEmiTex2 != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex2, 1);
	}
	if (uNormalTex2 != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, normalTexId);
		glUniform1i(uNormalTex2, 2);
	}
	if (uSpecTex2 != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_2D, specTexId);
		glUniform1i(uSpecTex2, 3);
	}

	// draw triangles
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

	// CUBE 3 (its going to have the same textures as the previous cube)
	// activate VAO with object configuration
	glBindVertexArray(vao3);
	// generate matrices for v shader
	modelView = view * model3;
	modelViewProj = proj * view * model3;
	normal = glm::transpose(glm::inverse(modelView));
	if (uModelViewMat2 != -1)
		glUniformMatrix4fv(uModelViewMat2, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat2 != -1)
		glUniformMatrix4fv(uModelViewProjMat2, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat2 != -1)
		glUniformMatrix4fv(uNormalMat2, 1, GL_FALSE, &(normal[0][0]));
	// draw triangles
	glDrawElements(GL_TRIANGLES, obj3TrianglesN * 3,
		GL_UNSIGNED_INT, (void*)0);

	glutSwapBuffers();
}

void resizeFunc(int width, int height) 
{
	glViewport(0, 0, width, height);
	
	proj = glm::perspective(glm::radians(60.0f), float(width) / float(height), 1.0f, 50.0f);

	// render event
	glutPostRedisplay();
}	

void idleFunc() 
{
	if (mainBifurcations == 0)
	{
		// object 1 movement
		model = glm::mat4(1.0f);
		static float angle = 0.0f;
		angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.0001f;
		model = glm::rotate(model, angle, glm::vec3(1.0f, 1.0f, 0.0f));

		// object 2 movement
		model2 = glm::mat4(1.0f);
		// for orbital movement
		glm::mat4 orbitRotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 traslation = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
		// other
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(-1.0f, 1.0f, 0.0f));
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

		model2 = orbitRotation * traslation * rotation * scale;

		// object 3 movement
		bezierMovement();
	} 
	else if (mainBifurcations == 1)
	{
		// object viewed as a plane from camera to see the anisotropic filter
		model = glm::mat4(1.0f);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(5.0));
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -7.5, -3.0));
		model = translate * scale * model;
	}
	
	// render event
	glutPostRedisplay();
}

void bezierMovement(){
	// Bezier is an aproximation algorithm, which means it only goes through first and las point, and aproximates the rest.
	// 4 control points, the last one in the same position as the first to make a loop
	// We need to use the parameter t to control the movement.

	// t parameter
	t = t + 0.00005f;
	if (t >= 1.0f) // go back to 0 to loop
		t = 0.0f;

	// Control points
	glm::vec2 P0 = glm::vec2(-5.0f, 0.0f);
	glm::vec2 P1 = glm::vec2(7.0f, 10.0f);
	glm::vec2 P2 = glm::vec2(7.0f, -10.0f);
	glm::vec2 P3 = P0;

	float B0 = pow((1.0f - t), 3.0f);
	float B1 = 3.0f * pow((1.0f - t), 2.0f) * t;
	float B2 = 3.0f * (1.0f - t) * pow(t, 2.0f);
	float B3 = pow(t, 3.0f);

	// Scale (make it smaller than the other ones to differenciate them)
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));

	// Bezier curve for m = 3 (this is a translation)
	float xAxis = B0 * P0.x + B1 * P1.x + B2 * P2.x + B3 * P3.x;
	float zAxis = B0 * P0.y + B1 * P1.y + B2 * P2.y + B3 * P3.y;
	glm::mat4 bezier = glm::translate(glm::mat4(1.0f), glm::vec3(xAxis, 0.0f, zAxis));

	model3 = bezier * scale;
}

void keyboardFunc(unsigned char key, int x, int y) {
	// Keyboard movement for camera
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

	// Light movement :)
	if (key == 'W')
		lightPos.z -= 0.1f;
	else if (key == 'S')
		lightPos.z += 0.1f;

	if (key == 'D')
		lightPos.x += 0.1f;
	else if (key == 'A')
		lightPos.x -= 0.1f;
	
	if (key == 'E')
		lightPos.y += 0.1f;
	else if (key == 'Q')
		lightPos.y -= 0.1f;

	if (key == '1')
		lightKeyboardSetting = 1;
	else if (key == '2')
		lightKeyboardSetting = 2;
	else if (key == '3')
		lightKeyboardSetting = 3;

	if(key == '+'){
		switch(lightKeyboardSetting){
			case 1:
				if (lightIa.x < 1.0) lightIa += 0.1f;
				break;
			case 2:
				if (lightIa.x < 1.0) lightId += 0.1f;
				break;	
			case 3:
				if (lightIa.x < 1.0) lightIs += 0.1f;
				break;
		}
	} else if (key == '-')
	{
		switch(lightKeyboardSetting){
			case 1:
				if (lightIa.x > 0.0) lightIa -= 0.1;
				break;
			case 2:
				if (lightId.x > 0.0)lightId -= 0.1;
				break;	
			case 3:
				if (lightIs.x > 0.0)lightIs -= 0.1;
				break;
		}
	}

	std::cout << "The pressed key is " << key << std::endl << std::endl;
}

void setViewMatGivenLookAtAndUp(){
	glm::vec3 n = -glm::normalize(lookAt);
	glm::vec3 v = glm::normalize(vUp - (n * vUp) * n);
	glm::vec3 u = glm::cross(v, n);

	glm::mat4 cameraView = glm::mat4(glm::vec4(u, 0), glm::vec4(v, 0), glm::vec4(n, 0), glm::vec4(COP, 1));
	view = glm::inverse(cameraView);
}

void mouseFunc(int button, int state, int x, int y) {
	if (state==0)
		std::cout << "The pressed button is ";
	else
		std::cout << "The released button is ";
	
	if (button == 0) std::cout << "left mouse button " << std::endl;
	if (button == 1) std::cout << "central mouse button " << std::endl;
	if (button == 2) std::cout << "right mouse button " << std::endl;

	std::cout << "in the window's position " << x << " " << y << std::endl << std::endl;
}

void mouseMotionFunc(int x, int y){	
	// the vector that we want to rotate is the want that goes from the camera to the object we orbit aroundv
	// that object is at (0,0,0) so the vector in this case is just the COP
	glm::vec3 camFocusVector = COP;

	// because we have another functionality moving the camera around the scene
	// and it can no longer look at the direction of the cube all the time
	// we always reset the lookAt to look at the cube when orbiting so the functionality makes sense
	lookAt = glm::normalize(-camFocusVector);

	float rotationSpeed = cameraYawPitchSpeed;
	if (x != lastXmouse){ // yaw (rotates camFocusVector around up vector) 
		if (x < lastXmouse) 
			rotationSpeed = -rotationSpeed;
		
		camFocusVector = glm::vec3(glm::rotate(glm::mat4(1.0f), rotationSpeed, vUp) * glm::vec4(camFocusVector, 0.0f));
		COP = camFocusVector;
		lookAt = glm::normalize(-camFocusVector);
	}
		
	if (y != lastYmouse){ // pitch (rotates camFocusVector around right vector)
		if (y < lastYmouse) 
			rotationSpeed = -rotationSpeed;

		glm::vec3 right = glm::normalize(cross(lookAt, vUp));
		camFocusVector = camFocusVector = glm::vec3(glm::rotate(glm::mat4(1.0f), rotationSpeed, right) * glm::vec4(camFocusVector, 0.0f));
		COP = camFocusVector;
		lookAt = glm::normalize(-camFocusVector);
		vUp = glm::normalize(cross(right, lookAt));
	} 

	// set mouse values for next call
	lastXmouse = x;
	lastYmouse = y;

	setViewMatGivenLookAtAndUp();
}

void initAssimp()
{
	// create and activate VAO
	glGenVertexArrays(1, &vao3);
	glBindVertexArray(vao3);

	// obtener modelo de assimp
	Assimp::Importer importer;
	std::string objPath = std::string(SHADERS_PATH) + "/../sphere.obj";
    const aiScene* scene = importer.ReadFile(objPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
	
	aiMesh* mesh = scene->mMeshes[0]; // load first mesh

    // triangle index because assimp stores them in faces and we need the whole array
	std::vector<unsigned int> assimpTriangleIndex;
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            assimpTriangleIndex.push_back(face.mIndices[j]);
    }

	obj3TrianglesN = (unsigned int)assimpTriangleIndex.size();

	std::vector<glm::vec3> tangents;
	tangents.resize(mesh->mNumVertices);
	std::vector<glm::vec3> normals;
	normals.resize(mesh->mNumVertices);

	// Calculate normals and tangents (from texture coordinates)
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];

		// Get triangle (face) vertices and UV coordinates
		aiVector3D& v0 = mesh->mVertices[face.mIndices[0]];
        aiVector3D& v1 = mesh->mVertices[face.mIndices[1]];
        aiVector3D& v2 = mesh->mVertices[face.mIndices[2]];

        aiVector3D& uv0 = mesh->mTextureCoords[0][face.mIndices[0]];
        aiVector3D& uv1 = mesh->mTextureCoords[0][face.mIndices[1]];
        aiVector3D& uv2 = mesh->mTextureCoords[0][face.mIndices[2]];

		// Get edges and deltaUV coordinates
        glm::vec3 edge1 = glm::vec3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
        glm::vec3 edge2 = glm::vec3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

        glm::vec2 deltaUV1 = glm::vec2(uv1.x - uv0.x, uv1.y - uv0.y);
        glm::vec2 deltaUV2 = glm::vec2(uv2.x - uv0.x, uv2.y - uv0.y);

        // Calculate tangent
        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);

		// Calculate normal as cross product of edges
		glm::vec3 normal = glm::cross(edge1, edge2);

		// For each vertex, we need the average of all tangents, we are gonna use the average based on the triangle area
		// area is half of the module of cross product of two edges (normal)
		float area = glm::length(normal) * 0.5f;
		tangent *= area;
		normal *= area;

		// store
		tangents[face.mIndices[0]] += tangent;
		tangents[face.mIndices[1]] += tangent;
		tangents[face.mIndices[2]] += tangent;
		
		normals[face.mIndices[0]] += normal;
		normals[face.mIndices[1]] += normal;
		normals[face.mIndices[2]] += normal;
    }

	float* rawTangents = new float[mesh->mNumVertices * 3];
	float* rawNormals = new float[mesh->mNumVertices * 3];
	float* rawColors = new float[mesh->mNumVertices * 3]; // porque el modelo no tiene colores en los vertices, inicializamos a rojo

	for (size_t i = 0; i < mesh->mNumVertices; ++i) {
		normals[i] = glm::normalize(normals[i]);
		// Gram-Schmidt orthogonalize
		tangents[i] = glm::normalize(tangents[i] - glm::dot(tangents[i], normals[i]) * normals[i]);		

        rawTangents[i * 3] = tangents[i].x;
        rawTangents[i * 3 + 1] = tangents[i].y;
        rawTangents[i * 3 + 2] = tangents[i].z;
		
        rawNormals[i * 3] = normals[i].x;
        rawNormals[i * 3 + 1] = normals[i].y;
        rawNormals[i * 3 + 2] = normals[i].z;

		rawColors[i * 3] = 1;
        rawColors[i * 3 + 1] = 0;
        rawColors[i * 3 + 2] = 0;
    }

	// create and configure mesh attributes
	glGenBuffers(1, &posVBO3);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO3);
		glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * sizeof(float) * 3,
			mesh->mVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
	
		glGenBuffers(1, &colorVBO3);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO3);
		glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * sizeof(float) * 3,
			rawColors, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &normalVBO3);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO3);
		glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * sizeof(float) * 3,
			mesh->mNormals, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glGenBuffers(1, &texCoordVBO3);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO3);
		glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * sizeof(float) * 2,
			mesh->mTextureCoords[0], GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);

		glGenBuffers(1, &tangentVBO3);
		glBindBuffer(GL_ARRAY_BUFFER, tangentVBO3);
		glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * sizeof(float) * 3,
			rawTangents, GL_STATIC_DRAW);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(4);
	
	glGenBuffers(1, &triangleIndexVBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, assimpTriangleIndex.size() * sizeof(unsigned int) * 3,
		assimpTriangleIndex.data(), GL_STATIC_DRAW);

	delete[] rawTangents;
	delete[] rawNormals;
	delete[] rawColors;
}