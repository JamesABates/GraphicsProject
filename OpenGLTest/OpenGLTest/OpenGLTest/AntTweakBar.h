#ifndef ANT_TWEAK_BAR_H
#define ANT_TWEAK_BAR_H

#include "./AntTweak/AntTweakBar.h"
#include <gl_core_4_4.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class AntTweakBar
{
public:
	AntTweakBar(GLFWwindow* window);
	~AntTweakBar();

	static void OnMouseButton(GLFWwindow*, int b, int a, int m);
	static void OnMousePosition(GLFWwindow*, double x, double y);
	static void OnMouseScroll(GLFWwindow*, double x, double y);
	static void OnKey(GLFWwindow*, int k, int s, int a, int m);
	static void OnChar(GLFWwindow*, unsigned int c);
	static void OnWindowResize(GLFWwindow*, int w, int h);

	void Draw();

	glm::vec4 m_clearColour;
	glm::vec3 m_light;
	glm::vec3 m_lightColour;
	float m_specPow;

private:
	TwBar* m_bar;
	GLFWwindow* m_window;
};

#endif