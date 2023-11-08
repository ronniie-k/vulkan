#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include "Renderer.h"
#include "utils/Log.h"

Camera::Camera()
{
	m_front = glm::vec3(0.f);
	m_right = glm::vec3(0.f);
	m_position = glm::vec3(0.f);;
	m_rotation = glm::vec2(0.f);;
	m_view = glm::mat4(1.f);
	m_proj = glm::mat4(1.f);

	updateView();
}

glm::mat4& Camera::getViewMatrix()
{
	return m_view;
}

glm::mat4& Camera::getProjMatrix()
{
	return m_proj;
}

void Camera::input(float dt)
{
	GLFWwindow* window = Renderer::getWindow();
	float speed = 0.05f;
	float sensitivity = 0.1f;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		m_position += m_front * speed * dt;

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		m_position -= m_front * speed * dt;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		m_position -= m_right * speed * dt;

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		m_position += m_right * speed * dt;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		m_position.y += speed * dt;

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		m_position.y -= speed * dt;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		updateLightPos = true;

	static glm::dvec2 lastMousePosition = {400, 300};
	glm::dvec2 mousePosition;
	glfwGetCursorPos(window, &mousePosition.x, &mousePosition.y);

	double xoffset = mousePosition.x - lastMousePosition.x;
	double yoffset = lastMousePosition.y - mousePosition.y;

	lastMousePosition = mousePosition;

	m_rotation.x += xoffset * sensitivity;
	m_rotation.y += yoffset * sensitivity;

	if (m_rotation.y > 89.f)
		m_rotation.y = 89.f;
	if (m_rotation.y < -89.f)
		m_rotation.y = -89.f;

	updateView();
}

void Camera::updateMatrices()
{
	updateView();
	updateProj();
}

void Camera::updateView()
{
	float cosPitch = glm::cos(glm::radians(m_rotation.y));
	glm::vec3 newFront;
	newFront.x = glm::cos(glm::radians(m_rotation.x)) * cosPitch;
	newFront.y = glm::sin(glm::radians(m_rotation.y));
	newFront.z = glm::sin(glm::radians(m_rotation.x)) * cosPitch;
	m_front = glm::normalize(newFront);
	m_right = glm::normalize(glm::cross(m_front, { 0.f, 1.f, 0.f }));
	m_view = glm::lookAt(m_position, m_position + m_front, { 0.f, 1.f, 0.f });
}

void Camera::updateProj()
{
	auto extent = Renderer::getSwapchainExtent();
	m_proj = glm::perspective(glm::radians(103.0f), extent.width / (float)extent.height, 0.1f, 1000.0f);
	m_proj[1][1] *= -1;
}
