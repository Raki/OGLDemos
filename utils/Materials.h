#pragma once
#include <glm/glm.hpp>
namespace Materials
{

	struct Material
	{
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		float shininess;
	};

	extern Material emerald,jade,obsidian,pearl,ruby,turquoise,brass,
		bronze,chrome,copper,gold,silver,black_plastic,cyan_plastic,green_plastic,
		red_plastic,white_plastic,yellow_plastic,black_rubber,cyan_rubber,
		green_rubber,red_rubber,white_rubber,yellow_rubber;
};

