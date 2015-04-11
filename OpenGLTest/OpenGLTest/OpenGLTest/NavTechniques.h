#ifndef NAV_TECHNIQUES_H
#define NAV_TECHNIQUES_H

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

struct NavNode
{
	glm::vec3 position;
	glm::vec3 vertices[3];
	NavNode* edgeTargets[3];

	// values usable by A* if you want
	unsigned int flags;
	float edgeCosts[3];
};

class NavTechniques
{
public:
	NavTechniques(FlyCamera* camera);
	~NavTechniques();
	void CreateOpenGLBuffers(FBXFile* fbx);
	void CreateShaders();
	void Update();
	void Draw();

private:
	FBXFile* m_sponza;
	FBXFile* m_navMesh;
	FlyCamera* m_camera;

	glm::mat4 m_modelWorld;
	GLfloat* m;

	std::vector<NavNode> m_graph;

	unsigned int m_vao;
	unsigned int m_vbo;
	unsigned int m_ibo;
	unsigned int m_fboDepth;
	unsigned int m_program;
};

#endif // !NAV_TECHNIQUES_H
