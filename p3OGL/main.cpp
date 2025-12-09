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
glm::mat4 model2 = glm::mat4(1.0f);

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
//Attributes
int inPos;
int inColor;
int inNormal;
int inTexCoord;
// - Textures
unsigned int colorTexId;
unsigned int emiTexId;


/// PROGRAM 2 ///
unsigned int vshader2;
unsigned int fshader2;
unsigned int program2;

//Attributes
int inPos2;
int inColor2;
int inNormal2;
int inTexCoord2;
// - Textures
unsigned int colorTexId2;
unsigned int emiTexId2;
//Uniform Variables
int uModelViewMat2;
int uModelViewProjMat2;
int uNormalMat2;
// - Uniform Textures
int uColorTex2;
int uEmiTex2;

//VAO. 1 VAO for each object. Each VAO has multiple VBO
unsigned int vao;

//VBOs for each VAO. Store different types of attributes for the object.
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

//VAO. 1 VAO for each object. Each VAO has multiple VBO
unsigned int vao2;

//VBOs for each VAO. Store different types of attributes for the object.
unsigned int posVBO2;
unsigned int colorVBO2;
unsigned int normalVBO2;
unsigned int texCoordVBO2;
unsigned int triangleIndexVBO2;

//////////////////////////////////////////////////////////////
// Auxiliar Functions
//////////////////////////////////////////////////////////////
// TO BE IMPLEMENTED

// CallBack declaration
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

// Initialization and destruction functions
void initContext(int argc, char** argv);
void initOGL();
void initShader1(const char* vname, const char* fname);
void initShader2(const char* vname, const char* fname);
void initObj1();
void initObj2();
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

	initContext(argc, argv);
	initOGL();
	std::string vertexShader = std::string(SHADERS_PATH) + "/shader.v0.vert";
	std::string fragmentShader = std::string(SHADERS_PATH) + "/shader.v0.frag";
	initShader1(vertexShader.c_str(), fragmentShader.c_str());
	vertexShader = std::string(SHADERS_PATH) + "/shader.v1.vert";
	fragmentShader = std::string(SHADERS_PATH) + "/shader.v1.frag";
	initShader2(vertexShader.c_str(), fragmentShader.c_str());

	initObj1();
	initObj2();

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

	// VAO VBO
	if (inPos != -1) glDeleteBuffers(1, &posVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1) glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1) glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);
	glDeleteVertexArrays(1, &vao);

	// textures
	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);

	// program 2	
	glDetachShader(program2, vshader2);
	glDetachShader(program2, fshader2);
	glDeleteShader(vshader2);
	glDeleteShader(fshader2);
	glDeleteProgram(program2);

	if (inPos2 != -1) glDeleteBuffers(1, &posVBO2);
	if (inColor2 != -1) glDeleteBuffers(1, &colorVBO2);
	if (inNormal2 != -1) glDeleteBuffers(1, &normalVBO2);
	if (inTexCoord2 != -1) glDeleteBuffers(1, &texCoordVBO2);	
	
	glDeleteBuffers(1, &triangleIndexVBO2);
	glDeleteVertexArrays(1, &vao2);

	// textures
	glDeleteTextures(1, &colorTexId2);
	glDeleteTextures(1, &emiTexId2);
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
	// idx for attributes
	inPos = glGetAttribLocation(program, "inPos");
	inColor = glGetAttribLocation(program, "inColor");
	inNormal = glGetAttribLocation(program, "inNormal");
	inTexCoord = glGetAttribLocation(program, "inTexCoord");
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
	// idx for attributes
	inPos2 = glGetAttribLocation(program2, "inPos");
	inColor2 = glGetAttribLocation(program2, "inColor");
	inNormal2 = glGetAttribLocation(program2, "inNormal");
	inTexCoord2 = glGetAttribLocation(program2, "inTexCoord");
}

void initObj1()
{
	// create and activate VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// create and configure mesh attributes
	if (inPos != -1) // check if attribute 'inPos' is linked to a program socket
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeNTriangleIndex * sizeof(unsigned int) * 3,
		cubeTriangleIndex, GL_STATIC_DRAW);

	// model matrix for this object
	model = glm::mat4(1.0f);

	// textures
	std::string colorTex = std::string(TEXTURES_PATH) + "/color2.png";
	std::string emissiveTex = std::string(TEXTURES_PATH) + "/emissive.png";
	colorTexId = loadTex(colorTex.c_str());
	emiTexId = loadTex(emissiveTex.c_str());
}

void initObj2()
{
	// create and activate VAO
	glGenVertexArrays(1, &vao2);
	glBindVertexArray(vao2);

	// create and configure mesh attributes
	if (inPos2 != -1) // check if attribute 'inPos2' is linked to a program socket
	{
		glGenBuffers(1, &posVBO2);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO2);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
			cubeVertexPos, GL_STATIC_DRAW);
		glVertexAttribPointer(inPos2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos2);
	}
	if (inColor2 != -1)
	{
		glGenBuffers(1, &colorVBO2);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO2);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
			cubeVertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(inColor2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor2);
	}
	if (inNormal2 != -1)
	{
		glGenBuffers(1, &normalVBO2);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO2);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
			cubeVertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal2);
	}
	if (inTexCoord2 != -1)
	{
		glGenBuffers(1, &texCoordVBO2);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO2);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2,
			cubeVertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord2);
	}
	glGenBuffers(1, &triangleIndexVBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeNTriangleIndex * sizeof(unsigned int) * 3,
		cubeTriangleIndex, GL_STATIC_DRAW);

	// model matrix for this object
	model = glm::mat4(1.0f);

	// textures
	std::string colorTex = std::string(TEXTURES_PATH) + "/color.png";
	std::string emissiveTex = std::string(TEXTURES_PATH) + "/emissive.png";
	colorTexId2 = loadTex(colorTex.c_str());
	emiTexId2 = loadTex(emissiveTex.c_str());
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
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // for zoom out, trilinear interpolation
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);				  // for zoom in, bilinear interpolation
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	return texId;
}

void renderFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// CUBE 1
	glUseProgram(program);

	// generate matrices for v shader
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));
	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,	&(normal[0][0]));

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
	}

	// activate VAO with object configuration
	glBindVertexArray(vao);
	// draw triangles
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

	// CUBE 2
	glUseProgram(program2);

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
		glBindTexture(GL_TEXTURE_2D, colorTexId2);
		glUniform1i(uColorTex2, 0);
	}
	if (uEmiTex2 != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId2);
		glUniform1i(uEmiTex2, 1);
	}

	// activate VAO with object configuration
	glBindVertexArray(vao2);
	// draw triangles
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

	glutSwapBuffers();
}

void resizeFunc(int width, int height)
{
	glViewport(0, 0, width, height);

	// render event
	glutPostRedisplay();
}

void idleFunc()
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

	// render event
	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y) {}
void mouseFunc(int button, int state, int x, int y) {}
