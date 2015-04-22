#ifndef APPLICATION_H
#define APPLICATION_H
//#define STB_IMAGE_IMPLEMENTATION

#include <gl_core_4_4.h>
#include <GLFW/glfw3.h>
#include <Gizmos.h>
//#include <stb/stb_image.h>
#include <string>

#include <iostream>
#include "Camera.h"
#include "FlyCamera.h"
#include "Texture.h"
#include "ObjectLoader.h"
#include "FBX/FBXFile.h"
#include "tinyobjloader-master\tiny_obj_loader.h"
#include "AntTweak\AntTweakBar.h"
#include "AntTweakBar.h"
#include "ParticleSystem.h"
#include "FBObject.h"
#include "ShadowMap.h"
#include "NavTechniques.h"
#include "ProceduralGen.h"

#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

class Camera;
class Texture;
class FlyCamera;
class ObjectLoader;
class FBXFile;
class ShadowMap;

using glm::vec3;
using glm::vec4;
using glm::mat4;

struct Vertex
{
	vec4 position;
	vec4 colour;
};


struct KeyFrame 
{
	glm::vec3 position;
	glm::quat rotation;
};

class Application
{
public:
	Application();
	~Application();

	void RunGame();
	void Update();
	void Draw();
	void generateGrid( unsigned int rows, unsigned int cols );
	void OBJLoader();
	void FBXLoader(const char* filename);
	void Controls();
	void CreateShaders();

private:
	mat4 view;
	mat4 projection;
	GLFWwindow* window;
	FlyCamera* camera;
	Texture* texture;
	Texture* texture2;
	ObjectLoader* objLoader;
	AntTweakBar* antTweakBar;
	FBObject* fbObject;
	ShadowMap* shadowMap;
	NavTechniques* navTechniques;
	ProcedualGen* procedualGen;
	std::string pyroFBX;
	std::string markFBX;

	glm::vec3 m_positions[2];
	glm::quat m_rotations[2];

	KeyFrame m_hipFrames[2];
	KeyFrame m_kneeFrames[2];
	KeyFrame m_ankleFrames[2];
	glm::mat4 m_hipBone;
	glm::mat4 m_kneeBone;
	glm::mat4 m_ankleBone;
	
	float currentTime;
	float deltaTime;
	float previousTime;

	float modelX, modelY, modelZ;

	float modelX2, modelY2, modelZ2;

	unsigned int VAO;
	unsigned int VBO;
	unsigned int IBO;

	unsigned int programID;

	unsigned int indexCount;
	
	float camX;
	float camY;
	float camZ;
	float rotation;

	//Texture
	const char* crate;
};

#endif