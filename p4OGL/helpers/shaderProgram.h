#pragma once
#include <GL/glew.h>
#define SOLVE_FGLUT_WARNING
#include <GL/freeglut.h>

class ShaderProgram
{
	public:
		unsigned int program;

		void Init(const char *vname, const char *fname, const char* attribs[]);
		void Use();
		void Destroy();  
	
	private:		
		unsigned int vShader;		
		unsigned int fShader;		

		GLuint loadShader(const char *fileName, GLenum type);
};