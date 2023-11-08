#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
	bool updateLightPos = false;
	Camera();

	glm::mat4& getViewMatrix();
	glm::mat4& getProjMatrix();

	glm::vec3& getPosition() { return m_position; }
	glm::vec2& getRotation() { return m_rotation; }

	void input(float dt);
	void updateMatrices();
private:
	void updateView();
	void updateProj();
private:
	glm::vec3 m_position;
	glm::vec2 m_rotation;
	glm::vec3 m_front;
	glm::vec3 m_right;

	glm::mat4 m_view;
	glm::mat4 m_proj;
};

