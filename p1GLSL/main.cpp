#include <IGL/BOX.h>
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
int obj3Id = -1;

// CB declaration
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);
void mouseMotionFunc(int x, int y);
void firstCubeMovement();
void secondCubeMovement();
void thirdCubeMovement();
void setViewMatGivenLookAtAndUp();
void assimpModelLoad();

// movement related variables and fixed update
float angle = 0.0f;
const float angleStep = glm::radians(90.0f);
auto  lastFrame = std::chrono::high_resolution_clock::now();
float deltaTimeAccumulator = 0.0f;
const float fixedDeltaTime = 1.0f / 60.0f;
float t = 0.0f;

// camera variables
glm::vec3 COP = glm::vec3(0.0f, 0.0f, 6.0f); // COP is the camera position
glm::vec3 lookAt = glm::vec3(0.0f, 0.0f, -1.0f); // camera orientation
glm::vec3 vUp = glm::vec3(0.0f, 1.0f, 0.0f); // camera's up vector
const float cameraMovementSpeed = 0.3f;
const float cameraRotationSpeed = glm::radians(10.0f);
const float cameraYawPitchSpeed = 0.05f;
int lastXmouse = 0;
int lastYmouse = 0;

int main(int argc, char** argv)
{
	#ifdef _WIN32
	std::locale::global(std::locale("spanish")); // Spanish accents
	#endif
	std::string vertexShader = std::string(SHADERS_PATH) + "/shader.vOb4.vert";
	std::string fragmentShader = std::string(SHADERS_PATH) + "/shader.vOp3.frag";
	if (!IGlib::init(vertexShader.c_str(), fragmentShader.c_str()))
		return -1;

	// Assimp import model
	assimpModelLoad();
   
	// Adjust the camera
	setViewMatGivenLookAtAndUp();

	// Create the object we want to visualize
	objId = IGlib::createObj(cubeNTriangleIndex, cubeNVertex, cubeTriangleIndex, 
			cubeVertexPos, cubeVertexColor, cubeVertexNormal,cubeVertexTexCoord, cubeVertexTangent);

	obj2Id = IGlib::createObj(cubeNTriangleIndex, cubeNVertex, cubeTriangleIndex, 
			cubeVertexPos, cubeVertexColor, cubeVertexNormal,cubeVertexTexCoord, cubeVertexTangent);

	//obj3Id = IGlib::createObj(cubeNTriangleIndex, cubeNVertex, cubeTriangleIndex, 
			//cubeVertexPos, cubeVertexColor, cubeVertexNormal,cubeVertexTexCoord, cubeVertexTangent);

	// Add textures here
	std::string texturePath = std::string(TEXTURES_PATH) + "/color.png";
	std::string texturePath2 = std::string(TEXTURES_PATH) + "/color2.png";
	std::string auxiliarTexturePath = std::string(TEXTURES_PATH) + "/alpha.png";
	IGlib::addColorTex(objId, texturePath.c_str());
	IGlib::addColorTex(obj2Id, texturePath2.c_str());
	IGlib::addAuxiliarTex(objId, auxiliarTexturePath.c_str());

  	// CBs
  	IGlib::setResizeCB(resizeFunc);
  	IGlib::setIdleCB(idleFunc);
  	IGlib::setKeyboardCB(keyboardFunc);
  	IGlib::setMouseCB(mouseFunc);
  	IGlib::setMouseMoveCB(mouseMotionFunc);
	
	// Main loop
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
		secondCubeMovement();
		thirdCubeMovement();

		deltaTimeAccumulator -= fixedDeltaTime;
	}
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
	if (state==0)
		std::cout << "The pressed button is ";
	else
		std::cout << "The released button is ";
	
	if (button == 0) std::cout << "left mouse button " << std::endl;
	if (button == 1) std::cout << "central mouse button " << std::endl;
	if (button == 2) std::cout << "right mouse button " << std::endl;

	std::cout << "in the window's position " << x << " " << y << std::endl << std::endl;
}

void mouseMotionFunc(int x, int y)
{
	// orbital camera functionality
	
	// the vector that we want to rotate is the want that goes from the camera to the object we orbit around
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

void firstCubeMovement(){
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 1.0f, 0.0f)); 
	IGlib::setModelMat(objId, model);
}

void secondCubeMovement(){
	// Because it is colMajor, model are all the tranformations applied to the object multiplied in inverse order
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)); // just so it is smaller than the first object 
	glm::mat4 yAxisRotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)); // rotates around itself
	glm::mat4 yAxisTranslatedRotation = yAxisRotation * glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f)); // first translate and then rotate (reuse yAxisRotation matrix because it rotates around the Y axis which is what we need again)
	glm::mat4 model = yAxisTranslatedRotation * yAxisRotation * scale;
	IGlib::setModelMat(obj2Id, model);
}

void thirdCubeMovement(){
	// Bezier is an aproximation algorithm, which means it only goes through first and las point, and aproximates the rest.
	// 4 control points, the last one in the same position as the first to make a loop
	// We need to use the parameter t to control the movement.

	// t parameter
	t = t + 0.005f;
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

	glm::mat4 model = bezier * scale;

	IGlib::setModelMat(obj3Id, model);
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

void assimpModelLoad(){
	Assimp::Importer importer;
	std::string objPath = std::string(SHADERS_PATH) + "/../suzanne.obj";
    const aiScene* scene = importer.ReadFile(objPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
	
	aiMesh* mesh = scene->mMeshes[0]; // load first mesh

    // triangle index because assimp stores them in faces and we need the whole array
	std::vector<unsigned int> assimpTriangleIndex;
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            assimpTriangleIndex.push_back(face.mIndices[j]);
    }

	obj3Id = IGlib::createObj(
		static_cast<unsigned int>(assimpTriangleIndex.size()),
		static_cast<unsigned int>(mesh->mNumVertices),
		reinterpret_cast<const unsigned int*>(assimpTriangleIndex.data()),
		reinterpret_cast<const float*>(mesh->mVertices),
		reinterpret_cast<const float*>(mesh->mColors),
		reinterpret_cast<const float*>(mesh->mNormals),
		reinterpret_cast<const float*>(mesh->mTextureCoords),
		reinterpret_cast<const float*>(mesh->mTangents)
	);
}