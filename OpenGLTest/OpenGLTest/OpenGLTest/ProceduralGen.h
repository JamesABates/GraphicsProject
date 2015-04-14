#ifndef PROCEDURAL_GEN_H
#define PROCEDURAL_GEN_H

#include <gl_core_4_4.h>
#include <GLFW/glfw3.h>

#include "FlyCamera.h"
#include "FBX/FBXFile.h"
#include "AntTweak\AntTweakBar.h"
#include "AntTweakBar.h"
#include <vector>

#include "stdlib.h"
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

struct Vertex2
{
	vec4 position;
	glm::vec2 texcoord;
	vec4 colour;
};

class ProcedualGen
{
public:
	ProcedualGen(FlyCamera* camera, GLFWwindow* window, AntTweakBar* gui);
	~ProcedualGen();

	void CreateShaders();
	void GenerateGrid(unsigned int rows, unsigned int cols);
	void GeneratePerlin();
	void GenerateOpenGLBuffers();
	void PerlinNoise();
	void Update(float dt);
	void Draw();

private:
	unsigned int m_program;
	unsigned int m_indexcount;
	unsigned int m_perlin_texture;
	unsigned int m_VAO;
	unsigned int m_VBO;
	unsigned int m_IBO;
	bool m_regenereate;

	int imageWidth = 0; 
	int imageHeight = 0; 
	int imageFormat = 0;

	unsigned char* data;

	unsigned int m_grass_texture;
	unsigned int m_water_texture;
	unsigned int m_rocks_texture;


	GLFWwindow* m_window;
	AntTweakBar* m_gui;
	FlyCamera* m_camera;
};

#endif