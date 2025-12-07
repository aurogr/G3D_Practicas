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


//VAO. 1 VAO for each object. Each VAO has multiple VBO
unsigned int vao;

//VBOs for each VAO. Store different types of attributes for the object.
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

//////////////////////////////////////////////////////////////
// New auxiliar variables

//Light attributes
int inLightPos;
int inLightIa;
int inLightId;
int inLightIs;

glm::vec3 lightPos = glm::vec3(0.0, 0.0, 0.0);
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
bool anistropicFilterOn = false;
GLfloat fLargest;

//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// Auxiliar Functions
void setViewMatGivenLookAtAndUp();
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
	std::string vertexShader = std::string(SHADERS_PATH) + "/shader.op1.vert";
	std::string fragmentShader = std::string(SHADERS_PATH) + "/shader.op1.frag";

	
	initShader(vertexShader.c_str(), fragmentShader.c_str());
	initObj();

	glutMainLoop();

	destroy();

	return 0;
}

//////////////////////////////////////////
// Auxiliar functions
void initContext(int argc, char **argv) 
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
	glutCreateWindow("Pr�cticas OGL");

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
	glEnable(GL_DEPTH_TEST);						// activate Z-test
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);			// define bg color
	glFrontFace(GL_CCW);							// front face orientation (clockwise = right hand, counter clockwise = left hand)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);		// configure rasterization
	glEnable(GL_CULL_FACE);							// activate culling

	// define matrices
	proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 50.0f); 
	view = glm::mat4(1.0f);
	view[3].z = -6;
}
void destroy() 
{
	// program 
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
}
void initShader(const char *vname, const char *fname) 
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
	// idx for attributes
	inPos = glGetAttribLocation(program, "inPos");
	inColor = glGetAttribLocation(program, "inColor");
	inNormal = glGetAttribLocation(program, "inNormal");
	inTexCoord = glGetAttribLocation(program, "inTexCoord");	
	// idx for light attributes
	inLightPos = glGetUniformLocation(program, "inLightPos");	
	inLightIa = glGetUniformLocation(program, "inLightIa");	
	inLightId = glGetUniformLocation(program, "inLightId");
	inLightIs = glGetUniformLocation(program, "inLightIs");
}
void initObj() 
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

GLuint loadShader(const char *fileName, GLenum type) 
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
unsigned int loadTex(const char *fileName) 
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

	glUseProgram(program);

	// generate matrices for v shader
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));
	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
			&(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
			&(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
			&(normal[0][0]));

	// generate light attributes for render
	if (inLightPos != -1) // check if attribute 'inPos' is linked to a program socket
	{
		glm::vec3 lightPosView = glm::vec3(view * glm::vec4(lightPos, 1));
		glUniform3fv(inLightPos, 1, &lightPosView[0]);
	}
	if (inLightIa != -1) // check if attribute 'inPos' is linked to a program socket
	{
		glUniform3fv(inLightIa, 1, &lightIa[0]);
	}
	if (inLightId != -1) // check if attribute 'inPos' is linked to a program socket
	{
		
		glUniform3fv(inLightId, 1, &lightId[0]);
	}
	if (inLightIs != -1) // check if attribute 'inPos' is linked to a program socket
	{
		
		glUniform3fv(inLightIs, 1, &lightIs[0]);
	}

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
	}

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
		// for object rotation
		model = glm::mat4(1.0f);
		static float angle = 0.0f;
		angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.001f;
		model = glm::rotate(model, angle, glm::vec3(1.0f, 1.0f, 0.0f));
	} 
	else if (mainBifurcations == 1)
	{
		// for object rotation
		model = glm::mat4(1.0f);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(5.0));
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -7.5, -3.0));
		model = translate * scale * model;
	}
	
	// render event
	glutPostRedisplay();

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
				if (lightIa.x < 0.9) lightIa += 0.1f;
				break;
			case 2:
				if (lightIa.x < 0.9) lightId += 0.1f;
				break;	
			case 3:
				if (lightIa.x < 0.9) lightIs += 0.1f;
				break;
		}
	} else if (key == '-')
	{
		switch(lightKeyboardSetting){
			case 1:
				if (lightIa.x > 0.1) lightIa -= 0.1;
				break;
			case 2:
				if (lightId.x > 0.1)lightId -= 0.1;
				break;	
			case 3:
				if (lightIs.x > 0.1)lightIs -= 0.1;
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

void mouseFunc(int button, int state, int x, int y) {}
