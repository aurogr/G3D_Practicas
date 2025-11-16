#include <IGL/IGlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/gl.h>
#define SOLVE_FGLUT_WARNING
#include <GL/freeglut.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include <fstream>

#include "OGLObject.h"

namespace IGlib
{
	//////////////////////////////////////////////////////////////
	// Global variables
	//////////////////////////////////////////////////////////////

	// Matrices
	glm::mat4 proj = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);

	// Background color
	glm::vec4 bgColor(0.0f);

	// Shaders
	unsigned int vshader;
	unsigned int fshader;
	unsigned int program;

	// Uniform matrix ID variables
	int uProjMat;
	int uModelViewMat;
	int uModelMat;
	int uViewMat;
	int uModelViewProjMat;
	int uNormalMat;

	// Uniform texture ID variables
	int uColorTex;
	int uEmiTex;
	int uNormalTex;
	int uSpecularTex;
	int uAuxiliarTex;

	// Attributes
	int inPos;
	int inColor;
	int inNormal;
	int inTangent;
	int inTexCoord;

	// Collection of objects to be drawn
	std::vector<OGLObject *> oGLObjectList;

	// CBs
	tResizeCB ResizeCB = NULL;
	tIdleCB IdleCB = NULL;
	tKeyboardCB KeyboardCB = NULL;
	tMouseCB MouseCB = NULL;
	tMouseMotionCB MouseMotionCB = NULL;

	////////////////////////////////////////
	// Auxiliar functions declaration
	////////////////////////////////////////
	void shaderInit(const char *vname, const char *fname);

	// GLUT CB
	void renderGlutEvent();
	void resizeGlutEvent(int width, int height);
	void keyboardGlutEvent(unsigned char key, int x, int y);
	void mouseGlutEvent(int button, int state, int x, int y);
	void mouseMotionGlutEvent(int x, int y);
	void idleGlutEvent();

	////////////////////////////////////////
	// Library functions
	////////////////////////////////////////
	bool init(const char *vertexShaderFileName, const char *fragmentShaderFileName)
	{
		// GLUT & context
		char *c[1] = {(char *)""};
		int n = 1;
		glutInit(&n, c);
		glutInitContextVersion(3, 3);
		glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
		glutInitContextProfile(GLUT_CORE_PROFILE);
		// glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

		// Framebuffer & Window
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
		glutInitWindowSize(500, 500);
		glutInitWindowPosition(0, 0);
		glutCreateWindow("G3D Exercises");

		// Extensions
		// Wait for OpenGL context creation
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			std::cout << "Error: " << glewGetErrorString(err) << std::endl;
			return false;
		}

		const GLubyte *oglVersion = glGetString(GL_VERSION);
		std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

		// Callbacks
		glutReshapeFunc(resizeGlutEvent);
		glutDisplayFunc(renderGlutEvent);
		glutIdleFunc(idleGlutEvent);
		glutKeyboardFunc(keyboardGlutEvent);
		glutMouseFunc(mouseGlutEvent);
		glutMotionFunc(mouseMotionGlutEvent);

		// OpenGL
		// Pipeline configuration
		glEnable(GL_DEPTH_TEST);
		glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);

		proj = glm::perspective(float(glm::radians(45.0f)), 1.0f, 0.1f, 50.0f);
		view = glm::mat4(1.0f);

		// Shaders
		shaderInit(vertexShaderFileName, fragmentShaderFileName);

		return true;
	}

	void destroy()
	{
		for (unsigned int i = 0; i < oGLObjectList.size(); i++)
			delete oGLObjectList[i];
		// for (auto obj : oGLObjectList) delete obj;

		oGLObjectList.clear();

		glDetachShader(program, vshader);
		glDetachShader(program, fshader);
		glDeleteShader(vshader);
		glDeleteShader(fshader);
		glDeleteProgram(program);
	}

	void mainLoop()
	{
		glutMainLoop();
	}

	void setProjMat(const glm::mat4 &projMat)
	{
		proj = projMat;
	}

	void setViewMat(const glm::mat4 &viewMat)
	{
		view = viewMat;
	}

	int createObj(unsigned int nTriangles, unsigned int nVertex,
				  const unsigned int *triangleIdx, const float *vertexPos,
				  const float *vertexColor, const float *vertexNormal,
				  const float *vertexTexCoord, const float *vertexTangent)
	{
		OGLObject *obj = new OGLObject;
		oGLObjectList.push_back(obj);
		obj->create(nTriangles, nVertex, triangleIdx,
					vertexPos, vertexColor, vertexNormal, vertexTexCoord, vertexTangent);

		return int(oGLObjectList.size()) - 1;
	}

	void setModelMat(int ID, const glm::mat4 &modelMat)
	{
		oGLObjectList[ID]->model = modelMat;
	}

	void addColorTex(int ID, const char *fileName)
	{
		oGLObjectList[ID]->loadColorTexture(fileName);
	}

	void addEmissiveTex(int ID, const char *fileName)
	{
		oGLObjectList[ID]->loadEmissiveTexture(fileName);
	}

	void addNormalTex(int ID, const char *fileName)
	{
		oGLObjectList[ID]->loadNormalTexture(fileName);
	}

	void addSpecularTex(int ID, const char *fileName)
	{
		oGLObjectList[ID]->loadSpecularTexture(fileName);
	}

	void addAuxiliarTex(int ID, const char *fileName)
	{
		oGLObjectList[ID]->loadAuxiliarTexture(fileName);
	}

	void setResizeCB(tResizeCB resizeCB)
	{
		ResizeCB = resizeCB;
	}

	void setIdleCB(tIdleCB idleCB)
	{
		IdleCB = idleCB;
	}

	void setKeyboardCB(tKeyboardCB keyboardCB)
	{
		KeyboardCB = keyboardCB;
	}

	void setMouseCB(tMouseCB mouseCB)
	{
		MouseCB = mouseCB;
	}

	void setMouseMoveCB(tMouseMotionCB mouseMotionCB)
	{
		MouseMotionCB = mouseMotionCB;
	}

	//////////////////////////////////////////////////////////////
	// Auxiliar functions
	//////////////////////////////////////////////////////////////

	// GLUT CB
	void resizeGlutEvent(int width, int height)
	{
		glViewport(0, 0, width, height);
		if (ResizeCB != NULL)
			ResizeCB(width, height);
		glutPostRedisplay();
	}

	void keyboardGlutEvent(unsigned char key, int x, int y)
	{
		if (keyboardGlutEvent != NULL)
			KeyboardCB(key, x, y);
		glutPostRedisplay();
	}

	void mouseGlutEvent(int button, int state, int x, int y)
	{
		int b = 0, s = 0;
		switch (button)
		{
		case GLUT_LEFT_BUTTON:
			b = 0;
			break;
		case GLUT_MIDDLE_BUTTON:
			b = 1;
			break;
		case GLUT_RIGHT_BUTTON:
			b = 2;
			break;
		default:
			b = 3;
			break;
		}

		if (state == GLUT_DOWN)
			s = 0;
		if (state == GLUT_UP)
			s = 1;

		if (MouseCB != NULL)
			MouseCB(b, s, x, y);
		glutPostRedisplay();
	}

	void mouseMotionGlutEvent(int x, int y)
	{
		if (MouseMotionCB != NULL)
			MouseMotionCB(x, y);
		glutPostRedisplay();
	}

	void idleGlutEvent()
	{
		if (IdleCB != NULL)
			IdleCB();
		glutPostRedisplay();
	}

	void renderGlutEvent()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);
		if (uProjMat != -1)
			glUniformMatrix4fv(uProjMat, 1, GL_FALSE, &(proj[0][0]));
		if (uViewMat != -1)
			glUniformMatrix4fv(uViewMat, 1, GL_FALSE, &(view[0][0]));

		// http://stackoverflow.com/questions/17705360/why-does-c-range-based-for-loop-calls-destructor-at-every-iteration
		// http://stackoverflow.com/questions/9331561/why-does-my-classs-destructor-get-called-when-i-add-instances-to-a-vector
		// for (auto obj : oGLObjectList) obj->render();
		for (unsigned int i = 0; i < oGLObjectList.size(); i++)
			oGLObjectList[i]->render();

		glUseProgram((unsigned int)NULL);
		glutSwapBuffers();
	}

	// Load shader functions
	char *loadStringFromFile(const char *fileName, unsigned int &fileLen)
	{
		// File loading
		std::ifstream file;
		file.open(fileName, std::ios::in);
		if (!file)
			return 0;

		// Compute file length
		file.seekg(0, std::ios::end);
		fileLen = (unsigned int)file.tellg();
		file.seekg(std::ios::beg);

		// Read the file
		char *source = new char[fileLen + 1];

		int i = 0;
		while (file.good())
		{
			source[i] = char(file.get());
			if (!file.eof())
				i++;
			else
				fileLen = i;
		}
		source[fileLen] = '\0';
		file.close();

		return source;
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
		delete source;

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
			std::cout << "Errors in " << fileName << std::endl;
			std::cout << logString << std::endl;
			delete logString;

			glDeleteShader(shader);
			return 0;
		}

		return shader;
	}

	void shaderInit(const char *vname, const char *fname)
	{
		// Compilation
		vshader = loadShader(vname, GL_VERTEX_SHADER);
		fshader = loadShader(fname, GL_FRAGMENT_SHADER);

		// Link
		program = glCreateProgram();
		glAttachShader(program, vshader);
		glAttachShader(program, fshader);

		glBindAttribLocation(program, 0, "inPos");
		glBindAttribLocation(program, 1, "inColor");
		glBindAttribLocation(program, 2, "inNormal");
		glBindAttribLocation(program, 3, "inTexCoord");
		glBindAttribLocation(program, 4, "inTangent");

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
			delete logString;

			glDeleteProgram(program);
			program = 0;
			return;
		}

		// Uniforms
		uProjMat = glGetUniformLocation(program, "proj");
		uViewMat = glGetUniformLocation(program, "view");
		uModelMat = glGetUniformLocation(program, "model");
		uNormalMat = glGetUniformLocation(program, "normal");
		uModelViewMat = glGetUniformLocation(program, "modelView");
		uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

		uColorTex = glGetUniformLocation(program, "colorTex");
		uEmiTex = glGetUniformLocation(program, "emiTex");
		uNormalTex = glGetUniformLocation(program, "normalTex");
		uSpecularTex = glGetUniformLocation(program, "specularTex");
		uAuxiliarTex = glGetUniformLocation(program, "auxiliarTex");

		// Attributes
		inPos = glGetAttribLocation(program, "inPos");
		inColor = glGetAttribLocation(program, "inColor");
		inNormal = glGetAttribLocation(program, "inNormal");
		inTexCoord = glGetAttribLocation(program, "inTexCoord");
		inTangent = glGetAttribLocation(program, "inTangent");
	}

}