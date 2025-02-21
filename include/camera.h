#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Camera
{
public:

	Camera();
	Camera(int width, int height, glm::vec3 up, float yaw, float pitch);
	~Camera();

	void setFov(float fov);
	void setDistance(float distance);

	glm::mat4 getViewMatrix() const { return glm::lookAt(m_position, m_target, m_up); }
	glm::mat4 getProjectionMatrix() const { return glm::perspective(glm::radians(m_fov), (float)m_width / (float)m_height, m_nearClippingPlane, m_farClippingPlane); }
	glm::vec3 getPosition() const { return m_position; }

	glm::vec3 getForward() const { return m_forward; }
	glm::vec3 getRight() const { return m_right; }
	glm::vec3 getUp() const { return m_up; }

	float getYaw() const { return m_yaw; }
	float getPitch() const { return m_pitch; }
	float getFov() const { return m_fov; }
	float getDistance() const { return m_distance; }

	void setPosition(glm::vec3 position);

	void updateCameraVectors();

	void orbit(float xoffset, float yoffset);

	void pan(float xoffset, float yoffset);

	void zoom(float yoffset);

	double lastX;
	double lastY;

private:
	unsigned int m_width, m_height;

	float m_sensivity = 0.1f;

	glm::vec3 m_position;
	glm::vec3 m_forward;
	glm::vec3 m_up;
	glm::vec3 m_right;
	glm::vec3 m_worldUp;

	glm::vec3 m_target;
	float m_distance = 10.0f;

	float m_yaw;
	float m_pitch;
	float m_fov = 60.0f;

	float m_farClippingPlane = 100.0f;
	float m_nearClippingPlane = 0.1f;

};