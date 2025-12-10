#include <IGL/BOX.h>
#include <IGL/suzanne.h>
#include <IGL/IGlib.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <chrono>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define PI 3.141592f

// Scene object IDs
int objId = -1;
int obj2Id = -1;

// CB declaration
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);
void mouseMotionFunc(int x, int y);
void firstCubeMovement();
void secondCubeMovement();
void setViewMatGivenLookAtAndUp();
void assimpModelLoad();

// movement related variables and fixed update
float angle = 0.0f;
const float angleStep = glm::radians(45.0f);
auto  lastFrame = std::chrono::high_resolution_clock::now();
float deltaTimeAccumulator = 0.0f;
const float fixedDeltaTime = 1.0f / 60.0f;

// camera variables
glm::vec3 COP = glm::vec3(0.0f, 0.0f, 6.0f); // COP is the camera position
glm::vec3 lookAt = glm::vec3(0.0f, 0.0f, -1.0f); // camera orientation
glm::vec3 vUp = glm::vec3(0.0f, 1.0f, 0.0f); // camera's up vector
const float cameraMovementSpeed = 0.3f;
const float cameraRotationSpeed = glm::radians(10.0f);
const float cameraYawPitchSpeed = 0.05f;
int lastXmouse = 0;
int lastYmouse = 0;
int mainBifurcations = 1; // 0: cube spinning (better for shaders that use textures)
						  // 1: cube spinning and suzanne orbitting (better for ilumination)
						  // 2: suzanne spinning (better for disney shader demonstrations)
						  // 3: suzanne uploaded from folder with calculated normals (for optional part 6)

int main(int argc, char **argv)
{
#ifdef _WIN32
	std::locale::global(std::locale("spanish")); // Spanish accents
#endif
	std::string vertexShader = std::string(SHADERS_PATH) + "/shader.ob1.vert";
	std::string fragmentShader = std::string(SHADERS_PATH) + "/shader.ob1.frag";
	if (!IGlib::init(vertexShader.c_str(), fragmentShader.c_str()))
		return -1;

	// Adjust the camera
	setViewMatGivenLookAtAndUp();
	
	switch(mainBifurcations){
		case 0:
			objId = IGlib::createObj(cubeNTriangleIndex, cubeNVertex, cubeTriangleIndex, cubeVertexPos, cubeVertexColor, cubeVertexNormal, cubeVertexTexCoord, cubeVertexTangent);
			break;
		case 1:
			objId = IGlib::createObj(cubeNTriangleIndex, cubeNVertex, cubeTriangleIndex, cubeVertexPos, cubeVertexColor, cubeVertexNormal, cubeVertexTexCoord, cubeVertexTangent);
			obj2Id = IGlib::createObj(SUZANNE_NUM_FACES, SUZANNE_NUM_VERTICES, suzanneFaces, suzanneVertexPos, fillSuzanneVertexColor(), suzanneVertexNormal, generateTriplanar(SUZANNE_NUM_VERTICES, 1.0f).data());
			break;
		case 2:
			objId = IGlib::createObj(SUZANNE_NUM_FACES, SUZANNE_NUM_VERTICES, suzanneFaces, suzanneVertexPos, fillSuzanneVertexColor(), suzanneVertexNormal, generateTriplanar(SUZANNE_NUM_VERTICES, 1.0f).data());
			break;
		case 3:
			assimpModelLoad();
			break;
	}	
	
	// Add textures here
	std::string albedoTexPath = std::string(TEXTURES_PATH) + "/color.png";
	std::string emissiveTexPath = std::string(TEXTURES_PATH) + "/emissive.png";
	std::string specMapTexPath = std::string(TEXTURES_PATH) + "/specMap.png";
	std::string normalTexPath = std::string(TEXTURES_PATH) + "/normal.png";
	IGlib::addColorTex(objId, albedoTexPath.c_str());
	IGlib::addEmissiveTex(objId, emissiveTexPath.c_str());
	IGlib::addSpecularTex(objId, specMapTexPath.c_str());
	IGlib::addNormalTex(objId, normalTexPath.c_str());

	// CBs
	IGlib::setIdleCB(idleFunc);
	IGlib::setResizeCB(resizeFunc);
	IGlib::setKeyboardCB(keyboardFunc);
	IGlib::setMouseCB(mouseFunc);
	IGlib::setMouseMoveCB(mouseMotionFunc);

	// Mainloop
	IGlib::mainLoop();
	IGlib::destroy();
	return 0;
}


void resizeFunc(int width, int height)
{
	// Adjust aspect ratio to window size

	// In Real Time Rendering equation 4.75 is said that a simpler setup is done with aspect ratio (a), vertical field of view (fovy), near (n) and far (f).
	// a = width/height
	// it uses parameter c, which is defined as c = 1.0/tan(fovy/2)
	
	// user-defined parameters
	float fovy = 60.0f;
	float n = 1.0;
	float f = 30.0;

	// calculated parameters
	float a = static_cast<float>(width)/height;
	float c = 1/tan(glm::radians(60.0f)/2);

	// projection matrix
	glm::mat4 simplerProj = glm::mat4(0.0);
	simplerProj[0][0] = c/a;
	simplerProj[1][1] = c;
	simplerProj[2][2] = -((f+n)/(f-n));
	simplerProj[2][3] = -1;
	simplerProj[3][2] = -((2*f*n)/(f-n));

	IGlib::setProjMat(simplerProj);
}

void idleFunc()
{
	// delta time for idle func
	auto currentFrame = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed = currentFrame - lastFrame;
	lastFrame = currentFrame;
	deltaTimeAccumulator += elapsed.count();

	while (deltaTimeAccumulator >= fixedDeltaTime){
		angle += angleStep * fixedDeltaTime;
		firstCubeMovement();

		if(mainBifurcations == 1)
			secondCubeMovement();

		deltaTimeAccumulator -= fixedDeltaTime;
	}
}

void firstCubeMovement(){
	glm::vec3 rotation = glm::vec3(1.0f, 1.0f, 0.0f);
	if (mainBifurcations == 2 || mainBifurcations == 3){ // rotate suzanne only on y axis
		rotation = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, rotation); 
	IGlib::setModelMat(objId, model);
}

void secondCubeMovement(){
	// Because it is colMajor, model are all the tranformations applied to the object multiplied in inverse order
	//glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)); // just so it is smaller than the first object 
	glm::mat4 yAxisRotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)); // rotates around itself
	glm::mat4 yAxisTranslatedRotation = yAxisRotation * glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f)); // first translate and then rotate (reuse yAxisRotation matrix because it rotates around the Y axis which is what we need again)
	glm::mat4 model = yAxisTranslatedRotation * yAxisRotation; //* scale;
	IGlib::setModelMat(obj2Id, model);
}

void setViewMatGivenLookAtAndUp(){
	// n = -lookAt/|lookAt| (k)
	// v = vUp' / |vUp'| --> vUp' = vUp - (n * vUp) * n (j)
	// u = v x n (i)
	glm::vec3 n = -glm::normalize(lookAt);
	glm::vec3 v = glm::normalize(vUp - (n * vUp) * n);
	glm::vec3 u = glm::cross(v, n);

	glm::mat4 cameraView = glm::mat4(glm::vec4(u, 0), glm::vec4(v, 0), glm::vec4(n, 0), glm::vec4(COP, 1));
	glm::mat4 view = glm::inverse(cameraView);
	IGlib::setViewMat(view);
}

void keyboardFunc(unsigned char key, int x, int y)
{ 	
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

	std::cout << "The pressed key is " << key << std::endl << std::endl;
}

void mouseFunc(int button, int state, int x, int y)
{
	if (state == 0)
		std::cout << "The pressed button is ";
	else
		std::cout << "The released button is ";

	if (button == 0)
		std::cout << "left mouse button " << std::endl;
	if (button == 1)
		std::cout << "central mouse button " << std::endl;
	if (button == 2)
		std::cout << "right mouse button " << std::endl;

	std::cout << "in the window's position " << x << " " << y << std::endl
			  << std::endl;
}

void mouseMotionFunc(int x, int y)
{
}

void assimpModelLoad(){
	Assimp::Importer importer;
	std::string objPath = std::string(SHADERS_PATH) + "/../suzanne.fbx";
    const aiScene* scene = importer.ReadFile(objPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
	
	aiMesh* mesh = scene->mMeshes[0]; // load first mesh

    // triangle index because assimp stores them in faces and we need the whole array
	std::vector<unsigned int> assimpTriangleIndex;
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            assimpTriangleIndex.push_back(face.mIndices[j]);
    }

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
    }
		

	objId = IGlib::createObj(
		static_cast<unsigned int>(assimpTriangleIndex.size()),
		static_cast<unsigned int>(mesh->mNumVertices),
		reinterpret_cast<const unsigned int*>(assimpTriangleIndex.data()),
		reinterpret_cast<const float*>(mesh->mVertices),
		reinterpret_cast<const float*>(mesh->mColors),
		//reinterpret_cast<const float*>(mesh->mNormals), // normals of assimp
		reinterpret_cast<const float*>(rawNormals), // calculated normals
		reinterpret_cast<const float*>(mesh->mTextureCoords),
		//reinterpret_cast<const float*>(mesh->mTangents) // tangents of assimp
		reinterpret_cast<const float*>(rawTangents) // calculated tangents
	);

	delete[] rawTangents;
	delete[] rawNormals;

	
	IGlib::setModelMat(objId, glm::mat4(1.0));
}