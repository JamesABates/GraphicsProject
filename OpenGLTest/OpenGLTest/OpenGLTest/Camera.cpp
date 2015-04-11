#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "Camera.h"

using glm::vec3;
using glm::vec4;
using glm::mat4;


Camera::Camera()
{
	UpdateProjectionViewTransform();
}

Camera::Camera(vec3 position)
{
	worldTransform[3] = vec4(position, 1);
	UpdateProjectionViewTransform();
}

Camera::Camera(mat4 transform) : worldTransform(transform)
{
	UpdateProjectionViewTransform();
}

void Camera::SetTransform(mat4 transform)
{
	worldTransform = transform;
	UpdateProjectionViewTransform();
}

const mat4 Camera::GetTransform() const
{
	return worldTransform;
}

void Camera::SetPosition(vec3 position)
{
	worldTransform[3] = vec4(position, 1);
	UpdateProjectionViewTransform();
}

void Camera::LookAt(vec3 lookAt, vec3 worldUp)
{
	vec4 vLocation = worldTransform[3];
	worldTransform = glm::inverse(glm::lookAt(vLocation.xyz(), lookAt, worldUp));
	UpdateProjectionViewTransform();
}

void Camera::LookAt(glm::vec3 position, glm::vec3 lookAt, glm::vec3 worldUp)
{
	worldTransform = glm::inverse(glm::lookAt(position, lookAt, worldUp));
	UpdateProjectionViewTransform();
}

void Camera::SetPerspective(float fov, float aspectRatio, float near, float far)
{
	projectionTransform = glm::perspective(fov, aspectRatio, near, far);
	
	UpdateProjectionViewTransform();
}

void Camera::UpdateProjectionViewTransform()
{
	viewTransform = glm::inverse(worldTransform);
	projectionViewTransform = projectionTransform * viewTransform;
}