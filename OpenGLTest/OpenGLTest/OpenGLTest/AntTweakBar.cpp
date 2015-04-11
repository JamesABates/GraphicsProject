#include "AntTweakBar.h"

AntTweakBar::AntTweakBar(GLFWwindow* window)
{
	m_window = window;
	m_specPow = 0;

	TwInit(TW_OPENGL_CORE, nullptr);
	TwWindowSize(1280, 720);

	glfwSetMouseButtonCallback(m_window, OnMouseButton);
	glfwSetCursorPosCallback(m_window, OnMousePosition);
	glfwSetScrollCallback(m_window, OnMouseScroll);
	glfwSetKeyCallback(m_window, OnKey);
	glfwSetCharCallback(m_window, OnChar);
	glfwSetWindowSizeCallback(m_window, OnWindowResize);

	m_bar = TwNewBar("my bar");

	TwAddVarRW(m_bar, "clear colour", TW_TYPE_COLOR4F, &m_clearColour[0], "");
	TwAddVarRW(m_bar, "light direction", TW_TYPE_DIR3F, &m_light[0], "group=light");
	TwAddVarRW(m_bar, "light colour", TW_TYPE_COLOR3F, &m_lightColour[0], "group=light");
	TwAddVarRW(m_bar, "spec power", TW_TYPE_FLOAT, &m_specPow, "group=light");
}

AntTweakBar::~AntTweakBar()
{
	TwDeleteAllBars();
	TwTerminate();
}

void AntTweakBar::Draw()
{
	TwDraw();
}

void AntTweakBar::OnMouseButton(GLFWwindow*, int b, int a, int m)
{
	TwEventMouseButtonGLFW(b, a);
}
 
void AntTweakBar::OnMousePosition(GLFWwindow*, double x, double y)
{
	TwEventMousePosGLFW((int)x, (int)y);
}

void AntTweakBar::OnMouseScroll(GLFWwindow*, double x, double y)
{
	TwEventMouseWheelGLFW((int)y);
}

void AntTweakBar::OnKey(GLFWwindow*, int k, int s, int a, int m)
{
	TwEventKeyGLFW(k, a);
}

void AntTweakBar::OnChar(GLFWwindow*, unsigned int c)
{
	TwEventCharGLFW(c, GLFW_PRESS);
}

void AntTweakBar::OnWindowResize(GLFWwindow*, int w, int h)
{
	TwWindowSize(w, h);
	glViewport(0, 0, w, h);
}