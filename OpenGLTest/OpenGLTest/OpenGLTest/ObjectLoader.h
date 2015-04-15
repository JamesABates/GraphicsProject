#pragma once
#include <vector>
#include <string>
#include "gl_core_4_4.h"
#include "glm\glm.hpp"
#include "tinyobjloader-master\tiny_obj_loader.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace std;

class FlyCamera;

struct OpenGLInfo
{
unsigned int m_VAO;
unsigned int m_VBO;
unsigned int m_IBO;
unsigned int m_index_count;
};

class ObjectLoader
{
public:

	ObjectLoader(FlyCamera* camera);
	~ObjectLoader();
	void Update();
	void Draw();
	void createOpenGLBuffers(std::vector<tinyobj::shape_t>& shapes);
	void CreateShader();
	void SetLightDir(float x, float y, float z) { m_lightDirX = x; m_lightDirY = y; m_lightDirZ = z; }
	void SetLightColour(float r, float g, float b) { m_lightR = r; m_lightG = g; m_lightB = b; }
	void SetSpecPow(float specPow) { m_specPow = specPow; }


private:

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::vector<OpenGLInfo> m_gl_info;
	unsigned int m_program_id;
	FlyCamera* m_camera;
	float m_lightDirX, m_lightDirY, m_lightDirZ;
	float m_lightR, m_lightG, m_lightB;
	float m_specPow;
	
	
};

