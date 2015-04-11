#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <gl_core_4_4.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

struct Particle
{
	glm::vec3	position;
	glm::vec3	velocity;
	glm::vec4	colour;
	float		size;
	float		lifetime;
	float		lifespan;
};

struct ParticleVertex
{
	glm::vec4	position;
	glm::vec4	colour;
};

class ParticleSystem
{
public:
	ParticleSystem();
	virtual ~ParticleSystem();

	void Update(float a_deltaTime, const glm::mat4& a_cameraTransform);
	void Draw();
	void CreateShaders();
	void initalise(unsigned int a_maxParticles, unsigned int a_emitRate, float a_lifetimeMin, float a_lifetimeMax, float a_velocityMin,
				   float a_velocityMax, float a_startSize, float a_endSize, const glm::vec4& a_startColour, const glm::vec4& a_endColour);
	void Emit();
	void SetPosition(glm::vec3 position) { m_position = position; }
	glm::vec3 GetPosition() { return m_position; }
	unsigned int m_program;
protected:

	Particle* m_particles;
	unsigned int m_firstDead;
	unsigned int m_maxParticles;

	unsigned int m_vao, m_vbo, m_ibo;
	ParticleVertex* m_vertexData;	glm::vec3 m_position;

	float m_emitTimer;
	float m_emitRate;

	float m_lifespanMin;
	float m_lifespanMax;

	float m_velocityMin;
	float m_velocityMax;

	float m_startSize;
	float m_endSize;

	glm::vec4 m_startColour;
	glm::vec4 m_endColour;

private:
};


#endif