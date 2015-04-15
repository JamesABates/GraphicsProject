#ifndef TEXTURE_H
#define TEXTURE_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <gl_core_4_4.h>
#include <GLFW/glfw3.h>
#include <Gizmos.h>
#include "FlyCamera.h"
#include "FBX\FBXFile.h"
#include "ParticleSystem.h"
#include "AntTweak\AntTweakBar.h"
#include "AntTweakBar.h"

class FlyCamera;

class Texture
{
struct Vertex 
{
	float x, y, z, w;
	float nx, ny, nz, nw;
	float tx, ty, tz, tw;
	float s, t;
};


public:
	Texture(FlyCamera* camera, AntTweakBar* gui);
	void Update(float dt);
	void Draw();
	void CreateShader();
	void createOpenGLBuffers(FBXFile* fbx);
	void Animations();
	void SetInputWindow(GLFWwindow* pWindow) { m_pWindow = pWindow; }
	glm::vec3 GetModelPosition() { return glm::vec3(modelX, modelY, modelZ); }
protected:
	
private:
	const char* m_image;
	unsigned char* m_data;
	unsigned int m_texture;
	unsigned int m_normalMap;
	unsigned int m_program;
	unsigned int m_vao;
	unsigned int m_vbo;
	unsigned int m_ibo;
	float modelX, modelY, modelZ;
	float m_timer;
	int m_width;
	int m_height;
	int m_format;
	bool crouch;
	enum animations {Idle, TakeDmgS, DeathS, Crouch, TakeDmgC, DeathC, Shoot, ReloadS, ReloadC, Run};
	animations aniNumber;
	glm::vec4 m_position;
	ParticleSystem* m_emitter;
	FlyCamera* m_camera;
	AntTweakBar* m_gui;
	FBXFile* m_fbxFile;
	FBXAnimation* animation;
	FBXSkeleton* skeleton;
	GLFWwindow* m_pWindow;
	glm::vec3 m_lightDirection;
};

#endif