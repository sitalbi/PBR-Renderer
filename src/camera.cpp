#include "camera.h"
#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
{
}

Camera::Camera(int width, int height, glm::vec3 up, float yaw, float pitch)
{
	m_width = width;
	m_height = height;
	m_worldUp = up;
	m_yaw = yaw;
	m_pitch = pitch;
	m_target = glm::vec3(0.0f);
	updateCameraVectors();
}

Camera::~Camera()
{
}


void Camera::setFov(float fov)
{
	m_fov = fov;
}

void Camera::setDistance(float distance) {
	m_distance = glm::max(distance, 0.1f);
	updateCameraVectors();
}

void Camera::setPosition(glm::vec3 position)
{
	m_position = position;
}

void Camera::updateCameraVectors() {
	glm::vec3 front;
	front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_forward = glm::normalize(front);

	m_right = glm::normalize(glm::cross(m_forward, m_worldUp));
	m_up = glm::normalize(glm::cross(m_right, m_forward));
	m_position = m_target - m_forward * m_distance;
}

void Camera::orbit(float xoffset, float yoffset)
{
	m_yaw += xoffset * m_sensivity;
	m_pitch += yoffset * m_sensivity;

	m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);

	updateCameraVectors();
}

void Camera::pan(float xoffset, float yoffset)
{
	m_position += (m_right * xoffset + m_up * yoffset) * m_sensivity;
	m_target += (m_right * xoffset + m_up * yoffset) * m_sensivity;
}

void Camera::zoom(float yoffset)
{
	m_distance -= yoffset;
	m_distance = glm::clamp(m_distance, 1.5f, 100.0f);
	updateCameraVectors();
}