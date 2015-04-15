#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include <gl_core_4_4.h>
#include <GLFW/glfw3.h>

#include "FBX/FBXFile.h"
#include "FlyCamera.h"
#include "AntTweak\AntTweakBar.h"
#include "AntTweakBar.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

class ShadowMap
{
public:
	ShadowMap(FlyCamera* flyCamera, AntTweakBar* gui);
	~ShadowMap();

	void CreateOpenGLBuffers(FBXFile* fbx);
	void SetupShadowBuffer();
	void CreateShaders();
	void CreateShadowShaders();

	void Update(float dt);
	void Draw();

private:
	unsigned int m_fbo;
	unsigned int m_fboDepth;
	unsigned int m_program;
	unsigned int m_shaderProgram;
	glm::mat4 m_lightMatrix;
	glm::vec3 m_lightDirection;
	unsigned int m_vao;
	unsigned int m_vbo;
	unsigned int m_ibo;

	AntTweakBar* m_gui;
	FBXFile* m_fbx;
	FlyCamera* m_flyCamera;
};

#endif