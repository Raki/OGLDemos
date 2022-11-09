#include <cstdlib>
#include "Colors.h"

namespace Color
{

	glm::vec3 red = glm::vec3(0.9, 0, 0);
	glm::vec3 green = glm::vec3(0, 0.9, 0);
	glm::vec3 blue = glm::vec3(0, 0, 0.9);
	glm::vec3 orange = glm::vec3(1, 0.49, 0.15);
	glm::vec3 brown = glm::vec3(0.72, 0.47, 0.34);
	glm::vec3 lightGreen = glm::vec3(0.7, 0.9, 0.11);
	glm::vec3 purple = glm::vec3(0.63, 0.28, 0.64);
	glm::vec3 pink = glm::vec3(1, 0.68, 0.78);
	glm::vec3 skyBlue = glm::vec3(0, 0.36, 0.72);
	glm::vec3 grey = glm::vec3(0.76, 0.76, 0.76);
	glm::vec3 white = glm::vec3(0.9f);
	glm::vec3 black = glm::vec3(0);

	glm::vec3 lightPink = glm::vec3(255 / 255.0, 174 / 255.0, 201 / 255.0);
	glm::vec3 lightOrange = glm::vec3(255 / 255.0, 201 / 255.0, 24 / 255.0);
	glm::vec3 cream = glm::vec3(239 / 255.0, 228 / 255.0, 176 / 255.0);
	glm::vec3 lightBlue = glm::vec3(153 / 255.0, 217 / 255.0, 234 / 255.0);
	glm::vec3 lightLavender = glm::vec3(200 / 255.0, 191 / 255.0, 231 / 255.0);

	glm::vec3 colrArr[] =
	{ red,green,blue,orange,lightGreen,purple,pink,brown,skyBlue,grey,white,lightPink,lightOrange,cream,lightBlue,lightLavender };

	int totalColors = sizeof(colrArr)/sizeof(glm::vec3);

	glm::vec3 getRandomColor()
	{
		int i = rand() % totalColors;

		return colrArr[i];
	}
};