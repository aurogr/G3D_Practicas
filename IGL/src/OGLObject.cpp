#include "OGLObject.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>

// Texture loading
#include <FreeImage.h>
#define _CRT_SECURE_DEPRECATE_MEMORY
#include <memory.h>

namespace IGlib
{

	// External variables
	// Matrices
	extern glm::mat4 proj;
	extern glm::mat4 view;

	// Uniform matrix ID variables
	extern int uModelViewMat;
	extern int uModelMat;
	extern int uModelViewProjMat;
	extern int uNormalMat;

	// Uniform texture ID variables
	extern int uColorTex;
	extern int uEmiTex;
	extern int uNormalTex;
	extern int uSpecularTex;
	extern int uAuxiliarTex;

	// Attributes
	extern int inPos;
	extern int inColor;
	extern int inNormal;
	extern int inTangent;
	extern int inTexCoord;

	// Auxiliar functions declaration
	unsigned char *loadTexture(const char *fileName, unsigned int &w, unsigned int &h);

	const unsigned int OGLObject::invalidValue = (unsigned int)-1;

	OGLObject::OGLObject() : vao(invalidValue), model(1.0f),
							 colorTexId(invalidValue),
							 emiTexId(invalidValue),
							 normalTexId(invalidValue),
							 specularTexId(invalidValue),
							 auxiliarTexId(invalidValue)
	{
	}

	OGLObject::~OGLObject()
	{
		destroy();
	}

	void OGLObject::destroy()
	{
		if (vao != invalidValue)
		{
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glDeleteBuffers(1, &posVBO);
			if (colorVBO != invalidValue)
				glDeleteBuffers(1, &colorVBO);
			if (normalVBO != invalidValue)
				glDeleteBuffers(1, &normalVBO);
			if (texCoordVBO != invalidValue)
				glDeleteBuffers(1, &texCoordVBO);
			if (tangentVBO != invalidValue)
				glDeleteBuffers(1, &tangentVBO);
			glDeleteBuffers(1, &triangleIndexVBO);

			glBindVertexArray(0);
			glDeleteVertexArrays(1, &vao);
		}
		if (colorTexId != invalidValue)
			glDeleteTextures(1, &colorTexId);
		if (emiTexId != invalidValue)
			glDeleteTextures(1, &emiTexId);
		if (normalTexId != invalidValue)
			glDeleteTextures(1, &normalTexId);
		if (specularTexId != invalidValue)
			glDeleteTextures(1, &specularTexId);
		if (auxiliarTexId != invalidValue)
			glDeleteTextures(1, &auxiliarTexId);
	}

	void OGLObject::render() const
	{
		if (vao == invalidValue)
			return;

		glm::mat4 modelView = view * model;
		glm::mat4 modelViewProj = proj * view * model;
		glm::mat4 normal = glm::transpose(glm::inverse(modelView));

		if (uModelMat != -1)
			glUniformMatrix4fv(uModelMat, 1, GL_FALSE, &(model[0][0]));
		if (uModelViewMat != -1)
			glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
		if (uModelViewProjMat != -1)
			glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
		if (uNormalMat != -1)
			glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

		// Textures
		if (uColorTex != -1 && colorTexId != invalidValue)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, colorTexId);
			glUniform1i(uColorTex, 0);
		}

		if (uEmiTex != -1 && emiTexId != invalidValue)
		{
			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, emiTexId);
			glUniform1i(uEmiTex, 1);
		}

		if (uNormalTex != -1 && normalTexId != invalidValue)
		{
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, normalTexId);
			glUniform1i(uNormalTex, 2);
		}

		if (uSpecularTex != -1 && specularTexId != invalidValue)
		{
			glActiveTexture(GL_TEXTURE0 + 3);
			glBindTexture(GL_TEXTURE_2D, specularTexId);
			glUniform1i(uSpecularTex, 3);
		}

		if (uAuxiliarTex != -1 && auxiliarTexId != invalidValue)
		{
			glActiveTexture(GL_TEXTURE0 + 4);
			glBindTexture(GL_TEXTURE_2D, auxiliarTexId);
			glUniform1i(uAuxiliarTex, 4);
		}

		// Buffer drawing
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, nTriangles, GL_UNSIGNED_INT, (void *)0);
	}

	void OGLObject::loadColorTexture(const char *fileName)
	{
		if (colorTexId != invalidValue)
			return;

		colorTexId = loadTex(fileName);
	}

	void OGLObject::loadEmissiveTexture(const char *fileName)
	{
		if (emiTexId != invalidValue)
			return;

		emiTexId = loadTex(fileName);
	}

	void OGLObject::loadNormalTexture(const char *fileName)
	{
		if (normalTexId != invalidValue)
			return;

		normalTexId = loadTex(fileName);
	}

	void OGLObject::loadSpecularTexture(const char *fileName)
	{
		if (specularTexId != invalidValue)
			return;

		specularTexId = loadTex(fileName);
	}

	void OGLObject::loadAuxiliarTexture(const char *fileName)
	{
		if (auxiliarTexId != invalidValue)
			return;

		auxiliarTexId = loadTex(fileName);
	}

	void OGLObject::create(unsigned int nTriangles, unsigned int nVertex,
						   const unsigned int *triangleIdx, const float *vertexPos,
						   const float *vertexColor, const float *vertexNormal,
						   const float *vertexTexCoord, const float *vertexTangent)
	{
		if (vao != invalidValue)
			return;

		this->nTriangles = nTriangles * 3;

		// VAO
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// Buffers
		glGenBuffers(1, &posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float) * 3, vertexPos, GL_STATIC_DRAW);
		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);

		if (inColor != -1 && vertexColor != NULL)
		{
			glGenBuffers(1, &colorVBO);
			glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
			glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float) * 3, vertexColor, GL_STATIC_DRAW);
			glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(inColor);
		}
		else
		{
			colorVBO = invalidValue;
		}

		if (inNormal != -1 && vertexNormal != NULL)
		{
			glGenBuffers(1, &normalVBO);
			glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
			glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float) * 3, vertexNormal, GL_STATIC_DRAW);
			glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(inNormal);
		}
		else
		{
			normalVBO = invalidValue;
		}

		if (inTexCoord != -1 && vertexTexCoord != NULL)
		{
			glGenBuffers(1, &texCoordVBO);
			glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
			glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float) * 2, vertexTexCoord, GL_STATIC_DRAW);
			glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(inTexCoord);
		}
		else
		{
			texCoordVBO = invalidValue;
		}

		if (inTangent != -1 && vertexTangent != NULL)
		{
			glGenBuffers(1, &tangentVBO);
			glBindBuffer(GL_ARRAY_BUFFER, tangentVBO);
			glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float) * 3, vertexTangent, GL_STATIC_DRAW);
			glVertexAttribPointer(inTangent, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(inTangent);
		}
		else
		{
			tangentVBO = invalidValue;
		}

		glGenBuffers(1, &triangleIndexVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, nTriangles * sizeof(unsigned int) * 3, triangleIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	unsigned int OGLObject::loadTex(const char *fileName)
	{
		// Texture file loading
		unsigned char *map;
		unsigned int w, h;
		map = loadTexture(fileName, w, h);

		if (!map)
		{
			std::cout << "Error loading file: " << fileName << std::endl;
			return invalidValue;
		}

		unsigned int texId;
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)map);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

		float aniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

		delete[] map;

		return texId;
	}

	//////////////////////
	// Auxiliar functions
	//////////////////////
	// Free the returned pointer
	unsigned char *loadTexture(const char *fileName, unsigned int &w, unsigned int &h)
	{
		FreeImage_Initialise(TRUE);

		FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fileName, 0);
		if (format == FIF_UNKNOWN)
			format = FreeImage_GetFIFFromFilename(fileName);
		if ((format == FIF_UNKNOWN) || !FreeImage_FIFSupportsReading(format))
			return NULL;

		FIBITMAP *img = FreeImage_Load(format, fileName);
		if (img == NULL)
			return NULL;

		FIBITMAP *tempImg = img;
		img = FreeImage_ConvertTo32Bits(img);
		FreeImage_Unload(tempImg);

		w = FreeImage_GetWidth(img);
		h = FreeImage_GetHeight(img);

		// BGRA to RGBA
		unsigned char *map = new unsigned char[4 * w * h];
		char *buff = (char *)FreeImage_GetBits(img);

		for (unsigned int j = 0; j < w * h; j++)
		{
			map[j * 4 + 0] = buff[j * 4 + 2];
			map[j * 4 + 1] = buff[j * 4 + 1];
			map[j * 4 + 2] = buff[j * 4 + 0];
			map[j * 4 + 3] = buff[j * 4 + 3];
		}

		FreeImage_Unload(img);
		FreeImage_DeInitialise();

		return map;
	}

	// http://stackoverflow.com/questions/9331561/why-does-my-classs-destructor-get-called-when-i-add-instances-to-a-vector
	// http://stackoverflow.com/questions/17705360/why-does-c-range-based-for-loop-calls-destructor-at-every-iteration
	OGLObject::OGLObject(OGLObject const &) //: nTriangles(T.),
	// posVBO(T.),
	// colorVBO(T.),
	// normalVBO(T.),
	// tangentVBO(T.),
	// texCoordVBO(T.),
	// triangleIndexVBO(T.),
	// colorTexId(T.),
	// emiTexId(T.),
	// normalTexId(T.),
	// specularTexId(T.),
	// model(T.)
	{
	}
}