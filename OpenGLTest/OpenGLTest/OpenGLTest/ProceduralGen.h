#ifndef PROCEDURAL_GEN_H
#define PROCEDURAL_GEN_H

#include <gl_core_4_4.h>
#include <GLFW/glfw3.h>

#include "FlyCamera.h"
#include "FBX/FBXFile.h"
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

struct Vertex2
{
	vec4 position;
	vec4 colour;
	vec4 texcoord;
};

class ProcedualGen
{
public:
	ProcedualGen(FlyCamera* camera);
	~ProcedualGen();

	void CreateShaders();
	void GenerateGrid(unsigned int rows, unsigned int cols);
	void PerlinNoise();
	void Update();
	void Draw();
private:
	unsigned int m_program;
	unsigned int m_indexcount;
	unsigned int m_VAO;
	unsigned int m_VBO;
	unsigned int m_IBO;

	FlyCamera* m_camera;
};

#endif