#ifndef CAMERA_H
#define CAMERA_H

#include <CommonHeaders.h>

class Camera
{
	
	glm::vec3 camDir,righDir;
	
	float camRadius = 1;
	bool lockCenter = true;
private:

	void update();
	
public:
	glm::vec3 eye, center, up;
	glm::mat4 viewMat;
	float camSpeed = 0.04f;
	Camera(glm::vec3 eye, glm::vec3 center, glm::vec3 up);
	void moveLeft();
	void moveRight();
	void moveForward();
	void moveBackward();
	void orbitY(float theta);
	void orbitX(float theta);
	void orbitXY(float theta, float phi);
	void zoomIn();
};


#endif // !CAMERA_H

