#pragma once

#include <glm/mat4x4.hpp>

struct ShaderMatrixInfo
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 normal;
	glm::vec4 lightPos;
	glm::vec4 cameraPos;
};