#include "Camera.h"

void Camera::update()
{
	camDir = eye - center;
	camRadius = glm::distance(eye, center);
	viewMat = glm::lookAt(eye, center, up);

	righDir = glm::cross(camDir,up);
}

Camera::Camera(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
{
	this->eye = eye;
	this->center = center;
	this->up = up;

	update();
}

void Camera::moveLeft()
{
	eye -= righDir * camSpeed;
	if(!lockCenter)
		center -= righDir * camSpeed;
	update();
}

void Camera::moveRight()
{
	eye += righDir * camSpeed;
	if (!lockCenter)
		center += righDir * camSpeed;
	update();
}

void Camera::moveForward()
{
	eye -= (camDir * camSpeed);
	if (!lockCenter)
		center -= (camDir * camSpeed);
	update();
}

void Camera::moveBackward()
{
	eye += (camDir * camSpeed);
	if (!lockCenter)
		center += (camDir * camSpeed);
	update();
}

void Camera::orbitY(float theta)
{
	auto x = center.x + (camRadius * cos(theta));
	auto z = center.z + (camRadius * sin(theta));
	eye.x = x;
	eye.z = z;
	//update();
	viewMat = glm::lookAt(eye, center, up);

	righDir = glm::cross(camDir, up);
}

void Camera::orbitX(float theta)
{
	auto y = center.y + (camRadius * cos(theta));
	auto z = center.z + (camRadius * sin(theta));
	eye.y = y;
	eye.z = z;
	//update();
	viewMat = glm::lookAt(eye, center, up);

	righDir = glm::cross(camDir, up);
}

void Camera::orbitXY(float theta, float phi)
{
	//x = rsin(theta)cos(phi)
	//y = r sin(theta)sin(phi)
	//z = rcos(theta)
	auto x = center.x + (camRadius * sin(theta) * cos(phi));
	auto y = center.y + (camRadius * sin(theta) * sin(phi));
	auto z = center.z + (camRadius * cos(theta));
	eye = glm::vec3(x, y, z);

	viewMat = glm::lookAt(eye, center, up);
	righDir = glm::cross(camDir, up);

}

void Camera::zoomIn()
{
}
