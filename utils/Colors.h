#pragma once
#include <glm/glm.hpp>
namespace Color
{
	extern glm::vec3 red;
	extern glm::vec3 green;
	extern glm::vec3 blue      ;
	extern glm::vec3 orange    ;
	extern glm::vec3 brown     ;
	extern glm::vec3 lightGreen;
	extern glm::vec3 purple    ;
	extern glm::vec3 pink      ;
	extern glm::vec3 skyBlue   ;
	extern glm::vec3 grey      ;
	extern glm::vec3 white     ;
	extern glm::vec3 black     ;
	extern glm::vec3 lightPink     ;
	extern glm::vec3 lightOrange   ;
	extern glm::vec3 cream         ;
	extern glm::vec3 lightBlue     ;
	extern glm::vec3 lightLavender ;
	extern glm::vec3 colrArr[];

	extern int totalColors;
	
	glm::vec3 getRandomColor();
	
};

