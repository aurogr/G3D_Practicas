#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace IGlib
{
	// Objects to be drawn
	class OGLObject
	{
		// Public methods
	public:
		OGLObject();
		~OGLObject();

		void destroy();
		void render() const;
		void loadColorTexture(const char *fileName);
		void loadEmissiveTexture(const char *fileName);
		void loadNormalTexture(const char *fileName);
		void loadSpecularTexture(const char *fileName);
		void loadAuxiliarTexture(const char *fileName);
		void create(unsigned int nTriangles, unsigned int nVertex,
					const unsigned int *triangleIdx, const float *vertexPos,
					const float *vertexColor = NULL, const float *vertexNormal = NULL,
					const float *vertexTexCoord = NULL, const float *vertexTangent = NULL);

		// Class attributes
	protected:
		// We use 0xFFFFFFFF non-valid value
		const static unsigned int invalidValue;

		// Object ID
		unsigned int vao;

		// 3D mesh triangles
		unsigned int nTriangles;

		// Attributes
		unsigned int posVBO;
		unsigned int colorVBO;
		unsigned int normalVBO;
		unsigned int tangentVBO;
		unsigned int texCoordVBO;
		unsigned int triangleIndexVBO;

		// Textures
		unsigned int colorTexId;
		unsigned int emiTexId;
		unsigned int normalTexId;
		unsigned int specularTexId;
		unsigned int auxiliarTexId;

	public:
		// Model matrix
		glm::mat4 model;

		// Protected methods
	protected:
		unsigned int loadTex(const char *fileName);

	private:
		// http://stackoverflow.com/questions/17705360/why-does-c-range-based-for-loop-calls-destructor-at-every-iteration
		// http://stackoverflow.com/questions/9331561/why-does-my-classs-destructor-get-called-when-i-add-instances-to-a-vector
		// OGLObject& operator=(OGLObject const&);
		OGLObject(OGLObject const &); // Copy
	};

}