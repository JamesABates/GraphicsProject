#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW\glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using glm::vec3;
using glm::vec4;
using glm::mat4;

class Camera
{
public:
	Camera();
	Camera(vec3 position);
	Camera(mat4 transform);

	virtual void Update(float dt){};

	void SetTransform(mat4 transform);
	const mat4 GetTransform() const;

	void SetPerspective(float fov, float aspectRatio, float near, float far);

	void LookAt(vec3 lookAt, vec3 up);
	void LookAt(vec3 position, vec3 lookAt, vec3 up);

	void SetPosition(vec3 position);
	vec3 GetPosition() const { return worldTransform[3].xyz(); }

	const glm::mat4& GetProjection() const { return projectionTransform; }
	const glm::mat4& GetView() const { return viewTransform; }
	const glm::mat4& GetProjectionView() const { return projectionViewTransform; }

	void UpdateProjectionViewTransform();
	bool GetPerspectiveSet() const { return bPerspectiveSet;  }
	
protected:
	mat4 worldTransform;
	mat4 viewTransform;
	mat4 projectionTransform;
	mat4 projectionViewTransform;

	bool bPerspectiveSet;
private:

	
};

#endif