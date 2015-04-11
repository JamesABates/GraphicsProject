#ifndef FLYCAMERA_H
#define FLYCAMERA_H

#include "Camera.h"
#include <GLFW\glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class FlyCamera : public Camera
{
public:

	FlyCamera()
	{
		//bViewButtonClicked = false;
		springiness = 50;
	}
	FlyCamera(float flySpeed, float rotSpeed) : Camera(), m_fFlySpeed(flySpeed), m_fRotationSpeed(rotSpeed)
	{
		springiness = 50;
	}
	~FlyCamera() {};

	void Update(double dt);

	void SetFlySpeed(float fSpeed);
	float GetFlySpeed() const { return m_fFlySpeed; }

	void SetInputWindow(GLFWwindow* pWindow) { m_pWindow = pWindow; }

protected:
	void HandleKeyboardInput(double dt);
	void HandleMouseInput(double dt);

	void CalculateRotation(double dt, double xOffset, double yOffset);

	GLFWwindow* m_pWindow;
	float m_fFlySpeed;
	float m_fRotationSpeed;
	float use_x, use_y;
	float springiness;
	bool m_bViewButtonClicked;
	double m_dCursorX, m_dCursorY;
};

#endif