
#include <gl_core_4_4.h>
#include <GLFW/glfw3.h>
#include <Gizmos.h>

#include "Camera.h"
#include "FlyCamera.h"
#include "Texture.h"
#include "AntTweak\AntTweakBar.h"
#include "AntTweakBar.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

class FlyCamera;

class FBObject
{
public:
	FBObject(FlyCamera* camera, GLFWwindow* pWindow, AntTweakBar* gui);
	void Update(float dt);
	void Draw();
	void CreateShaders();
	void CreateFrameBuffers();

private:
	unsigned int m_fbo;
	unsigned int m_fboTexture;
	unsigned int m_fboDepth;
	unsigned int m_vao;
	unsigned int m_vbo;
	unsigned int m_ibo;
	unsigned int m_program;

	Texture* m_texture;
	FlyCamera* m_camera;
};